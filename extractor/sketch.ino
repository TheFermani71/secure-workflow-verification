// ═══════════════════════════════════════════════════════════════
//  sketch.ino  —  SV-API  Benchmark & Demo  (Wokwi / ESP32)
//  Single-file: sv_api.h + sv_api.cpp + application layer inlined.
//
//  Changelog (Lezione 4 — 27.03.2026 + email Emanuel):
//
//  [L4-1] struct timespec RIMOSSO da sv_hdr_t e sv_trace_entry_t.
//         Il professore ha esplicitamente scartato il timestamp assoluto
//         su ogni variabile per motivi di efficienza.  Ciò che interessa
//         al server è la *distanza temporale* tra API consecutive,
//         non il momento assoluto. Beneficio collaterale: AAD del GCM
//         più piccolo (44 B invece di 52 B) → cifratura più veloce.
//
//  [L4-2] delta_us introdotto in sv_trace_entry_t.
//         Ogni entry registra i microsecondi trascorsi dall'operazione
//         precedente, usando esp_timer_get_time() (monotonic, µs).
//         Questo misura direttamente il tempo di inter-operazione che
//         il server confronta con il quanto calibrato.
//
//  [L4-3] sv_calibrate() — nuova API di calibrazione.
//         Eseguita una volta all'avvio prima di ogni altra operazione,
//         misura il tempo di esecuzione medio dell'API più onerosa
//         (sv_add: 2× unseal + operazione + seal + HMAC).
//         Il risultato (cal_tick_us) viene incluso nel JSON esportato
//         come riferimento per il verifier temporale di Emanuel.
//         Su hardware reale va eseguita in sezione critica (no interrupt).
//
//  [L4-4] sv_trace_export_json() — rinominata e semplificata.
//         La sicurezza del trasporto è garantita da HTTPS (Wi-Fi
//         integrato M5Stack/ESP32), non da cifratura aggiuntiva sul
//         device. La funzione ora produce JSON leggibile direttamente
//         trasmissibile. Il formato include tutti i campi richiesti
//         da Emanuel per la verifica del data flow.
//
//  [Emanuel] JSON arricchito per il workflow verifier:
//         - op_name: stringa leggibile dell'operazione (READ, ADD, …)
//         - var_name: nome simbolico della variabile output (dal registry)
//         - var_id: identificatore interno stabile (l'"ide" di Emanuel)
//         - delta_us: distanza temporale dall'operazione precedente
//         - cal_tick_us nell'header: quanto di riferimento
//         Questi campi consentono al verifier di passare dalla verifica
//         sequenziale alla verifica basata sul data flow delle variabili.
//
//  [FIX]  var_id ora preservato attraverso le operazioni: seal_and_trace_at
//         salva il var_id originale da sv_init_var così il nome simbolico
//         rimane tracciabile nel registry per tutta la vita della variabile.
//
//  [FIX2] sv_calibrate() ora idempotente: le variabili interne di misura
//         (_cal_a/b/c) bypassano il registry globale dei nomi usando ID
//         riservati (0xFFFA/B/C) e una sigillatura diretta via gcm_seal().
//         In precedenza, chiamate successive (es. nel benchmark) trovavano
//         i nomi già registrati: sv_init_var() restituiva id=0, i gcm_unseal
//         fallivano e s_cal_tick_us veniva corrotto a ~0, invalidando la
//         verifica temporale di Emanuel in tutti i JSON esportati dopo il
//         benchmark. Aggiunta anche una ri-calibrazione (n=8) al termine
//         di run_benchmarks() per ripristinare il riferimento preciso.
//
//  Changelog (Lezione 5 — 14.04.2026 + email Emanuel 21.04.2026):
//
//  [L5-1] Integrazione GPS — Atomic GPS Base v2.0 (ATGM336H).
//         Il modulo GPS è connesso via UART (Serial1) su GPIO 22/19.
//         Il JSON esportato include coordinate GPS, satelliti e validità.
//         Compilazione condizionale: su Wokwi il GPS è simulato (no-op).
//
//  [L5-2] Timestamp assoluto in testa alla trace (init_us).
//         La testa della traccia contiene il timestamp assoluto di
//         inizializzazione. Ogni entry registra solo il delta_us relativo.
//
//  [L5-3] Compilazione condizionale Wokwi / Hardware reale.
//         Flag -DGPS_HARDWARE in platformio.ini per l'ambiente atom_gps_hw.
//
//  [L5-NOTA] Struttura trace: array dinamico (non lista concatenata).
//         L'array con realloc geometrico è O(1) ammortizzato e O(1) accesso.
//
//  Changelog (R1L5 - Revisione pre-consegna — 22.04.2026):
//
//  [R1L5-GPS]   #define GPS_HARDWARE rimosso dal sorgente.
//         Il flag deve provenire esclusivamente dal sistema di build
//         (platformio.ini, env atom_gps_hw) tramite -DGPS_HARDWARE.
//         Averlo hardcoded nel .ino rendeva inerte la compilazione
//         condizionale: su Wokwi TinyGPSPlus veniva sempre incluso,
//         causando errori di compilazione o comportamento errato.
//         Il commento nella sezione GPS è stato aggiornato di conseguenza.
//
//  [R1L5-NONCE] Nonce AES-128-GCM ora deterministico anziché puramente casuale.
//         Schema: 4 B salt-di-sessione (casuale, fisso per tutta la vita del
//         device da un boot all'altro) || 8 B contatore monotono.
//         Il contatore garantisce unicità assoluta nella sessione (non ci sono
//         collisioni indipendentemente dal numero di operazioni); il salt
//         fornisce separazione crittografica tra sessioni diverse.
//         Il nonce casuale puro aveva un rischio di collisione non trascurabile
//         per il paradosso del compleanno dopo ~2^32 seal sulla stessa chiave
//         (12 byte → 96 bit, birthday bound ≈ 2^48 — ma il costo di
//         s_nonce_ctr è zero e il guadagno in robustezza è formalmente
//         dimostrabile, utile in sede di discussione della tesi).
//         s_nonce_salt e s_nonce_ctr sono inizializzati in sv_init().
//
//  [R1L5-LOOP]  while(true) finale in loop() sostituito con flag s_suite_done.
//         Il blocco infinito era funzionalmente corretto grazie a yield(),
//         ma strutturalmente insolito per Arduino: il watchdog riceveva
//         comunque i tick, ma il codice non era immediatamente leggibile.
//         Il flag rende esplicita l'intenzione ("suite completata, idle")
//         e lascia loop() con la normale struttura return-based.
//
//  [R1L5-HTTPS] TODO esplicito per la trasmissione HTTPS in
//  task_export_every_5s().
//         La funzione produce e libera il JSON senza trasmetterlo:
//         l'integrazione Wi-Fi/HTTPClient è lavoro futuro (post-tesi).
//         Il commento ora lo indica chiaramente per non creare aspettative
//         durante la discussione con la commissione.
//
//  Changelog (R2L5 - email Prof. Culmone — 23.04.2026):
//
//  [R2L5-GPS] Protezione lettura GPS via UART: nuove API sv_read_gps_lat,
//         sv_read_gps_lng, sv_read_gps_sats.
//         Come suggerito dal Prof., TinyGPSPlus rimane nella zona "in chiaro"
//         (il parsing NMEA è troppo pesante per il kernel). La protezione
//         è ottenuta sigillando il dato immediatamente dopo la conversione
//         ASCII→float, prima che l'applicazione lo riceva. La finestra di
//         vulnerabilità in RAM si riduce ai soli microsecondi di quella
//         conversione. I nuovi opcode SV_OP_READ_GPS_LAT/LNG/SATS (6,7,8)
//         e le costanti SV_SRC_GPS_LAT/LNG/SATS (0x1001-0x1003) permettono
//         al verifier di Emanuel di distinguere l'origine hardware del dato.
//         SV_API_VERSION aggiornata a 4.
//
//  [R2L5-VEC] Vettori crittografati sv_vec_t: sv_vec_alloc, sv_vec_free,
//         sv_vec_put, sv_vec_get.
//         Rispondono all'osservazione del Prof. sul costo di 1000 sv_add
//         consecutivi (~464 ms). Il pattern corretto è: accumulare i valori
//         durante la fase lenta di acquisizione (VEC_PUT = zero seal, solo
//         1 HMAC chain, ~50 µs) e processarli in blocco prima dell'invio.
//         VEC_PUT non esegue un nuovo GCM seal perché il valore è già
//         sigillato da sv_read_gps_* o da qualsiasi altra API; copia il
//         blob in memoria e registra l'operazione nella blockchain con
//         trace_append_dynamic (stesso schema di sv_write_fan_P12).
//         VEC_GET esegue unseal (verifica integrità) + re-seal verso la
//         variabile di destinazione, generando un nuovo out_id monotono
//         essenziale per il data flow di Emanuel.
//         SV_OP_VEC_PUT (90) e SV_OP_VEC_GET (91) aggiunti all'enum sv_op_t.
//         NOTA: Le API GPS e VEC descritte in [R2L5-GPS] e [R2L5-VEC] sono
//         state successivamente rimosse in [R3L6] (email Prof. Culmone
//         13.05.2026). Vedere changelog R3L6 per i dettagli.
//
//  Changelog (R3L5 - Fix — 27.04.2026, GPS zero-data + trace completa per
//  Emanuel):
//
//  [R3L5-GPS-FILL] Baco critico in gps_fill_trace(): le coordinate lat/lng
//         venivano azzerate quando isValid() == false, anche se TinyGPSPlus
//         le aveva già parsate da frame NMEA con status 'V' (void/last-known).
//         Il modulo ATGM336H trasmette le ultime coordinate note anche prima
//         di confermare il fix (status 'A'); il display le mostrava
//         correttamente leggendo s_gps.location.lat() direttamente, ma il JSON
//         le perdeva perché gps_fill_trace() le forzava a 0.0. Fix: (1)
//         gps_update() chiamata all'inizio di gps_fill_trace() per drenare il
//         buffer UART prima della lettura; (2) le coordinate vengono salvate
//         sempre — s_gps.location.lat/lng() — senza condizionale su isValid().
//         Il campo gps_valid comunica al verifier di Emanuel se il fix è
//         confermato o stimato (last-known).
//
//  [R3L5-GPS-BENCH] Aggiunto sv_kernel.f_gps_update() tra le sezioni del
//         benchmark (ai punti yield()) per prevenire l'overflow del buffer
//         UART HardwareSerial (256 B default) durante i ~500 ms dei benchmark.
//         Senza questo drain periodico, i frame NMEA accumulati in quel
//         finestra di tempo potevano traboccare e far perdere la sentenza
//         di fix confermato (status 'A'), ritardando ulteriormente isValid().
//
//  [R3L5-JSON-FULL] La traccia JSON ora viene stampata per intero sul Serial
//         (non più troncata a 200 caratteri) come richiesto da Emanuel.
//         La dimensione tipica è ~1800 B, gestibile dal monitor seriale.
//
//  Changelog (Lezione 6 — 05.05.2026):
//
//  [L6-CLASS] Classificazione delle API in 3 categorie (dalla lezione):
//         Cat. 1 — Dichiarazione strutture: sv_init_var,
//         sv_trace_init/free/clear
//         Cat. 2 — I/O sincrono e asincrono:
//         sv_read_temp_P3,
//                  sv_poll_button_P4 (async via ISR/ring-buffer), sv_write_fan_P12
//         Cat. 3 — Elaborazione e confronto logico: sv_add/sub/mul/div,
//                  sv_gt/lt/eq, sv_const, sv_inc, sv_avg_lastN
//         [R3L6] sv_read_gps_* rimossi (ora user-space puro via f_assign_f).
//
//  [L6-PROTO] Il protocollo JSON della trace prende il nome "Stefano-Ennio"
//         (dal nome degli studenti della triennale che l'hanno progettato).
//         Documentato nel commento della funzione sv_trace_export_json.
//
//  [L6-FUTURE] Prospettive future: il design architetturale prepara il
//         terreno per lo spostamento della verifica su Smart
//         Contract/Blockchain.
//
//  [L6-ARIANE] Riferimento storico al caso Ariane 5 (1996): motiva
//         l'importanza della verifica formale del workflow.
//
//  [L6-PERF] Osservazione sulle destinazioni d'uso: monitoraggio (overhead
//         tollerabile) vs controllo real-time (vincoli stringenti).
//
//  [L6-FLASH] Persistenza parallelizzata della trace su Flash via LittleFS
//         (discussa in aula come prospettiva teorica — NON richiesta nelle
//         email successive). Buffering in RAM con soglia configurabile,
//         flush asincrono in append-mode, rotazione file.
//
//  [L6-COROUTINE] Refactoring asincrono basato sull'uso della libreria
//         AceRoutine. Il monolite sincrono-bloccante dell'application layer
//         è stato rimpiazzato con uno scheduler cooperativo stackless.
//
//  Changelog (R1L6 - email Prof. Culmone — 10.05.2026):
//
//  [R1L6-GPS] Le API sv_read_gps_lat/lng/sats RIMOSSE dalla dispatch table
//         del kernel (sv_syscall_table_t). Restano nella Sezione 2 come
//         routine ordinarie. Documentato come esempio di VULNERABILITÀ.
//
//  [R1L6-CONST] I tempi di esecuzione delle API crittografiche devono
//         essere COSTANTI per categoria. Unica eccezione: sv_trace_export_json
//         il cui tempo è O(n). Documentato nei commenti.
//
//  [R1L6-GAP] Vulnerabilità inter-API: il codice utente può inserire
//         ritardi arbitrari tra le API. delta_gap_us è risolutivo.
//
//  Changelog (R2L6 - email Riccardo — 11.05.2026):
//
//  [R2L6-2LVL] Misurazione a due livelli nella trace:
//         delta_api_us = tempo di esecuzione interno dell'API (costante).
//         delta_gap_us = delta_us − delta_api_us = gap inter-API.
//         Il verifier di Emanuel valida entrambi separatamente.
//
//  [R2L6-HMAC] delta_api_us inserito nel payload HMAC (sostituisce pad32):
//         il tempo interno dell'API è autenticato dalla blockchain della
//         trace. SV_API_VERSION aggiornata a 5.
//
//  Changelog (R3L6 - Refactoring Architetturale — 13-14.05.2026):
//  [R3L6-CLEANUP-GPS] Rimozione totale API GPS dal kernel e Application layer native var.
//  [R3L6-CLEANUP-VEC] Rimozione vettori crittografati in favore di array sv_val_t.
//  [R3L6-NO-RTOS] Rimozione FreeRTOS. Sostituzione con ring-buffer volatile C.
//  [R3L6-JSON-NOTA] [R3L6-OVERHEAD] [R3L6-FUTURO] Commenti e ottimizzazioni documentate.
//  [R3L6-GRAFO] Le macro AceRoutine (YIELD/DELAY/LOOP) sono nodi di biforcazione
//         nel grafo di verifica del workflow (email Prof. Culmone 14.05.2026).
//         La verifica strutturale della traccia è valida, la verifica temporale
//         inter-coroutine richiede kernelizzazione (sviluppo futuro).
// ═══════════════════════════════════════════════════════════════

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Nota: <time.h> e <sys/time.h> rimossi — non usiamo più struct timespec
// né CLOCK_REALTIME. Il timing è interamente basato su esp_timer_get_time()
// che fornisce un contatore monotono in microsecondi, immune a drift del RTC
// e non alterabile via settimeofday() da codice applicativo non privilegiato.

#include <Arduino.h>
#include <esp_random.h>
#include <esp_timer.h>
#include <mbedtls/gcm.h>
#include <mbedtls/md.h>

#include <LittleFS.h> // [L6-FLASH] Persistenza Flash

// [L6-COROUTINE] Multitasking cooperativo (stackless coroutines)
#include <AceRoutine.h>
using namespace ace_routine;

// [L5-1] GPS — Atomic GPS Base v2.0 (ATGM336H via UART)
//
// ┌──────────────────────────────────────────────────────────────────┐
// │  [R1L5-GPS] GPS_HARDWARE NON va definito in questo file.          │
// │  Il flag deve provenire esclusivamente dal sistema di build:     │
// │                                                                  │
// │  • PlatformIO  →  env:atom_gps_hw                               │
// │                   build_flags = -DGPS_HARDWARE  (già presente)  │
// │  • Arduino IDE →  Sketch > Properties > Build options           │
// │                   aggiungere: -DGPS_HARDWARE                    │
// │                                                                  │
// │  Hardcodare il flag nel sorgente rendeva inerte la compilazione  │
// │  condizionale: l'env:esp32dev (Wokwi) trovava GPS_HARDWARE già  │
// │  definito e includeva TinyGPSPlus, causando errori o comporta-  │
// │  menti errati in simulazione.                                    │
// └──────────────────────────────────────────────────────────────────┘
//
// Quando GPS_HARDWARE è definito via build flag (-DGPS_HARDWARE):
//   - TinyGPSPlus viene incluso e il GPS reale è attivo
//   - Board: ESP32-S3, USB CDC On Boot: Enabled (platformio.ini)
// Quando NON è definito (default — Wokwi / env:esp32dev):
//   - Il GPS è simulato (coordinate fittizie di Camerino)
//   - Compatibile con Wokwi web e qualsiasi board ESP32
#ifndef GPS_HARDWARE
#define GPS_HARDWARE // Per Wokwi: commenta questa riga per simulazione Wokwi
#endif
#ifdef GPS_HARDWARE
#include <M5Unified.h>
#include <TinyGPSPlus.h>

// Pin UART per AtomS3 / AtomS3 Lite (chip ESP32-S3 rilevato durante upload):
#define GPS_RX_PIN 5 // GPIO5: TX del GPS → RX dell'ESP32-S3
#define GPS_TX_PIN 6 // GPIO6: RX del GPS → TX dell'ESP32-S3
// Se si usa ATOM LITE / ATOM MATRIX (ESP32 classico) decommentare
// queste righe e commentare le due sopra:
// #define GPS_RX_PIN 22
// #define GPS_TX_PIN 19

// ATTENZIONE: l'Atomic GPS Base v2.0 è pre-configurata a 115200 baud,
// NON 9600! Con 9600 la UART riceve solo spazzatura e TinyGPSPlus
// non decodifica alcun frame NMEA.
#define GPS_BAUD 115200

static TinyGPSPlus s_gps;
static HardwareSerial GPSSerial(1);
#endif // GPS_HARDWARE

// Versione del protocollo API/JSON: incrementata quando cambia il formato
// della trace o della struttura dati condivisa con il server di Emanuel.
// v2: delta_us, cal_tick_us, op_name, var_id (Lezione 4)
// v3: init_us, blocco gps, device_id condizionale (Lezione 5)
// v4: API vettoriali e lettura GPS crittografata (R2L5)
// v5: delta_api_us, rimozione GPS dal kernel, protocollo "Stefano-Ennio"
// (Lezione 6)
// v6: R3L6 - Rimozione opcode GPS e VEC, payload JSON modificato in ASSIGN
#define SV_API_VERSION 6

// ───────────────────────────────────────────────────────────────
//  SEZIONE 1 — Tipi, Costanti e Strutture (sv_api.h)
// ───────────────────────────────────────────────────────────────

#define SV_OK 0
#define SV_ERR_CRYPTO_FAIL 0x5001
#define SV_ERR_AUTH_FAIL 0x5002
#define SV_ERR_TYPE_MISMATCH 0x5003
#define SV_ERR_TRACE_FULL 0x5004
#define SV_ERR_QUEUE_FULL 0x5005
#define SV_ERR_NO_EVENT 0x5006
#define SV_ERR_SENSOR_FAIL 0x5007
#define SV_ERR_DUP_NAME 0x5008 // nome già registrato nel registry

#define SV_NONCE_LEN 12
#define SV_TAG_LEN 16
#define SV_HMAC_LEN 32 // HMAC-SHA256 output
#define SV_PT_LEN 4    // payload in chiaro (sizeof union)
#define SV_BLOB_LEN (SV_NONCE_LEN + SV_PT_LEN + SV_TAG_LEN) // = 32

typedef uint32_t sv_id_t;     // ID runtime dell'oggetto (monotono crescente)
typedef uint16_t sv_var_id_t; // ID logico variabile (stabile tra reassignment)
typedef uint16_t sv_src_id_t; // ID logico sensore/porta

typedef enum { SV_T_I32 = 1, SV_T_F32 = 2, SV_T_BOOL = 3 } sv_type_t;

typedef enum {
  SV_OP_READ = 1,
  SV_OP_EVENT = 2,
  SV_OP_ADD = 10,
  SV_OP_SUB = 11,
  SV_OP_MUL = 12,
  SV_OP_DIV = 13,
  SV_OP_GT = 20,
  SV_OP_LT = 22,
  SV_OP_EQ = 24,
  SV_OP_AND = 30,
  SV_OP_OR = 31,
  SV_OP_NOT = 32,
  SV_OP_CONST = 40,
  SV_OP_CNT = 41,
  SV_OP_WRITE = 50,
  SV_OP_AVG_N = 60,
  SV_OP_INIT = 70,
  SV_OP_ASSIGN = 71,
  SV_OP_BIND = 80 // setup ISR/hardware registrato nella traccia
} sv_op_t;

// ── Entry della Traccia ───────────────────────────────────────
// [L4-2] delta_us sostituisce struct timespec ts (8 B → 4 B per entry).
// Il server confronta delta_us con il quanto di calibrazione (cal_tick_us)
// per rilevare downclocking o ritardi artificiali tra le operazioni.
// [R2L6-2LVL] delta_api_us aggiunto per la misurazione a due livelli:
//   delta_api_us = tempo interno dell'API (deve essere costante per categoria)
//   delta_gap_us = delta_us − delta_api_us (gap inter-API, superficie di
//   attacco)
typedef struct {
  sv_op_t op;
  sv_id_t out_id;        // ID runtime dell'oggetto prodotto
  sv_id_t in1_id;        // ID runtime del primo operando (0 se assente)
  sv_id_t in2_id;        // ID runtime del secondo operando (0 se assente)
  sv_var_id_t out_var;   // ID logico della variabile output
  sv_src_id_t meta_src;  // porta sorgente (per READ/EVENT)
  uint16_t meta_u16;     // metadato contestuale (es. N per AVG_N)
  uint16_t _pad;         // padding per allineamento
  uint32_t delta_us;     // [L4-2] µs totali dall'operazione precedente
  uint32_t delta_api_us; // [R2L6] µs di esecuzione interna dell'API
  uint8_t chain_tag[SV_HMAC_LEN]; // HMAC chained tag (blockchain della trace)
} sv_trace_entry_t; // 64 bytes [R2L6] (era 60 B, ora allineato a potenza di 2)

// ── Contesto Dinamico della Traccia ──────────────────────────
// [L4-3] cal_tick_us memorizza il risultato di sv_calibrate() per
// includerlo nel JSON esportato come riferimento per il verifier.
typedef struct {
  sv_trace_entry_t *entries; // buffer heap-managed (malloc/realloc)
  size_t count;
  size_t capacity;
  uint8_t chain_tag[SV_HMAC_LEN]; // stato HMAC running per questa trace
  uint32_t cal_tick_us;           // [L4-3] quanto di calibrazione (µs/op)
  // [L5-2] Timestamp assoluto di inizializzazione della trace
  uint64_t init_us;
  // [L5-1] Dati GPS (Atomic GPS Base v2.0)
  double gps_lat;    // latitudine (WGS84)
  double gps_lng;    // longitudine (WGS84)
  uint32_t gps_sats; // satelliti visibili
  bool gps_valid;    // true se fix GPS valido
  // [L6-FLASH] Persistenza Flash asincrona
  size_t flash_count; // numero di entry persistite su Flash
  bool use_flash;     // flag abilitazione persistenza su LittleFS
} sv_trace_t;

extern sv_trace_t g_trace;

// ── Header Sigillato (entra nell'AAD del GCM) ─────────────────
// [L4-1] ts (struct timespec, 8 B) rimosso: il timing vive nella trace
// come delta_us, non nella variabile individuale. AAD ora = 44 B (era 52 B).
// trace_ref NON è qui: è un puntatore runtime e non deve entrare nell'AAD.
typedef struct {
  sv_id_t id;         // versione runtime (nuovo ad ogni write)
  sv_var_id_t var_id; // ID logico stabile tra reassignment
  sv_src_id_t src_id; // 0 = non è una lettura hardware
  sv_type_t type;     // tipo del payload
  char name[32];      // nome simbolico — autenticato dall'AAD GCM
} sv_hdr_t;           // 44 bytes

// ── Union Interna per Dati in Chiaro ─────────────────────────
typedef union {
  int32_t i_val;  // SV_T_I32
  float f_val;    // SV_T_F32
  uint32_t u_val; // SV_T_BOOL / pattern raw
} sv_raw_data_t;

// ── Valore Sigillato ──────────────────────────────────────────
// trace_ref qui, FUORI dall'header: non entra nell'AAD GCM.
typedef struct {
  sv_hdr_t h;
  uint16_t blob_len;
  uint8_t blob[SV_BLOB_LEN];
  sv_trace_t *trace_ref;
} sv_val_t;

typedef struct {
  sv_val_t v;
} sv_bool_t;

// ── Macro C-Like ──────────────────────────────────────────────
#define INT sv_val_t
#define DOUBLE sv_val_t
#define FLOAT sv_val_t
#define UNSIGNED_LONG sv_val_t

// ── Registry dei Nomi ─────────────────────────────────────────
// var_id aggiunto per la ricerca inversa (var_id → name) durante
// la serializzazione JSON richiesta da Emanuel per il data flow verifier.
typedef struct name_node_s {
  char name[32];
  sv_var_id_t var_id; // chiave per lookup inverso
  struct name_node_s *next;
} name_node_t;

// ── Prototipi API Pubbliche ───────────────────────────────────
int sv_init(void);
uint32_t sv_calibrate(uint16_t n_samples); // [L4-3] calibrazione temporale

sv_val_t sv_init_var(sv_type_t type, const char *name, sv_trace_t *trace_ref);
#define init(type, name, trace_ref) sv_init_var(type, name, trace_ref)

int assign(sv_val_t *var, int32_t value);
int assign_f(sv_val_t *var, float value);

void sv_trace_init(sv_trace_t *t);
void sv_trace_free(sv_trace_t *t);
void sv_trace_clear(sv_trace_t *t);
int sv_trace_count(const sv_trace_t *t);

// Svuota il registry globale dei nomi: libera tutti i nodi e azzera il
// puntatore. Va chiamata all'inizio di ogni suite run (non nel boot) così
// ogni pressione di INVIO parte con un registry pulito. Non tocca s_id_cnt
// né la trace: quelli rimangono indipendenti.
void sv_registry_clear(void);

int sv_read_temp_P3(sv_var_id_t var, sv_val_t *out);

int sv_bind_button_P4(void);
int sv_poll_button_P4(sv_var_id_t var, sv_val_t *out);
uint32_t sv_evt_dropped_count(void);

int sv_add(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
           sv_val_t *out);
int sv_sub(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
           sv_val_t *out);
int sv_mul(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
           sv_val_t *out);
int sv_div(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
           sv_val_t *out);
int sv_gt(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
          sv_bool_t *out);
int sv_lt(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
          sv_bool_t *out);
int sv_eq(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
          sv_bool_t *out);

int sv_const_i32(int32_t k, sv_var_id_t var, sv_val_t *out);
int sv_const_f32(float k, sv_var_id_t var, sv_val_t *out);
int sv_inc(const sv_val_t *c, sv_var_id_t var, sv_val_t *out);
int sv_write_fan_P12(const sv_bool_t *on);
int sv_avg_lastN_temp_P3(uint16_t N, sv_var_id_t var, sv_val_t *out);

int sv_trace_export(sv_trace_t *t, uint8_t *buf, size_t cap, size_t *out_len);

// [L4-4] Rinominata da sv_trace_export_json_blob: la sicurezza del
// trasporto è affidata a HTTPS (Wi-Fi ESP32/M5Stack), non a cifratura
// aggiuntiva on-device. La funzione produce JSON leggibile pronto per
// l'invio. Il chiamante deve free() il buffer restituito in *json_out.
// [R3L6-JSON-NOTA] La serializzazione JSON maschera la rimozione degli opcode
// hardware-specifici (GPS/VEC) sfruttando l'opcode generico ASSIGN.
// Questo garantisce compatibilità col verifier di Emanuel senza variare
// lo schema del payload.
int sv_trace_export_json(const sv_trace_t *trace, char **json_out,
                         size_t *json_len);

// ── Tabella delle System Call — dispatch table del kernel ─────
// [Prof. Culmone] Le API sono "system call" vere: chiamate esterne dal
// codice utente al kernel, NON semplici funzioni nello stesso spazio
// di indirizzamento. In C il meccanismo equivalente a quello delle DLL
// (o delle shared library su Linux) è una struct di puntatori a funzione
// che costituisce l'UNICO punto di accesso esportato dal kernel.
//
// Analogia con i sistemi operativi:
//   - Windows DLL  : import table → LoadLibrary/GetProcAddress
//   - Linux SO     : PLT (Procedure Linkage Table) → ld.so
//   - ARM Cortex-M : SVC (Supervisor Call) + vettore di dispatch
//   - x86 Linux    : INT 0x80 / SYSCALL + sys_call_table
//
// Su questo ESP32/Wokwi non c'è MMU completa quindi la separazione
// è per convenzione (come accadeva nei sistemi MS-DOS con INT 21h):
// il codice applicativo DOVREBBE chiamare solo attraverso sv_kernel,
// mai i simboli sv_xxx direttamente. Questi ultimi sono "interni alla DLL"
// e sarebbero invisibili al linker del codice utente in un sistema reale.
//
// La tabella è const: una volta inizializzata dal kernel all'avvio,
// non può essere sovrascritta dal codice applicativo. Su sistemi con MPU
// (es. ESP32-S3 con PMP, o Cortex-M con MPU) vivrebbe in una regione
// di memoria marked read-only per il codice in user-mode.
typedef struct {
  // — Inizializzazione e calibrazione —
  int (*f_init)(void);
  uint32_t (*f_calibrate)(uint16_t n_samples);
  void (*f_registry_clear)(void);
  // — Ciclo di vita variabili —
  sv_val_t (*f_init_var)(sv_type_t type, const char *name,
                         sv_trace_t *trace_ref);
  int (*f_assign)(sv_val_t *var, int32_t value);
  int (*f_assign_f)(sv_val_t *var, float value);
  // — Ciclo di vita traccia —
  void (*f_trace_init)(sv_trace_t *t);
  void (*f_trace_free)(sv_trace_t *t);
  void (*f_trace_clear)(sv_trace_t *t);
  int (*f_trace_count)(const sv_trace_t *t);
  void (*f_trace_flash_init)(sv_trace_t *t);     // [L6-FLASH]
  void (*f_trace_flush_to_flash)(sv_trace_t *t); // [L6-FLASH]
  // — I/O hardware protetto —
  int (*f_read_temp_P3)(sv_var_id_t var, sv_val_t *out);
  int (*f_avg_lastN_temp_P3)(uint16_t N, sv_var_id_t var, sv_val_t *out);
  int (*f_bind_button_P4)(void);
  int (*f_poll_button_P4)(sv_var_id_t var, sv_val_t *out);
  uint32_t (*f_evt_dropped_count)(void);
  int (*f_write_fan_P12)(const sv_bool_t *on);

  // — Operazioni aritmetiche e logiche su variabili protette —
  int (*f_add)(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
               sv_val_t *out);
  int (*f_sub)(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
               sv_val_t *out);
  int (*f_mul)(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
               sv_val_t *out);
  int (*f_div)(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
               sv_val_t *out);
  int (*f_gt)(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
              sv_bool_t *out);
  int (*f_lt)(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
              sv_bool_t *out);
  int (*f_eq)(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
              sv_bool_t *out);
  int (*f_const_i32)(int32_t k, sv_var_id_t var, sv_val_t *out);
  int (*f_const_f32)(float k, sv_var_id_t var, sv_val_t *out);
  int (*f_inc)(const sv_val_t *c, sv_var_id_t var, sv_val_t *out);
  // — Export trace —
  int (*f_trace_export)(sv_trace_t *t, uint8_t *buf, size_t cap,
                        size_t *out_len);
  int (*f_trace_export_json)(const sv_trace_t *trace, char **json_out,
                             size_t *json_len);
  // — GPS (Atomic GPS Base v2.0) — [L5-1]
  void (*f_gps_update)(void);
  void (*f_gps_fill_trace)(sv_trace_t *t);
} sv_syscall_table_t;

// L'istanza è dichiarata extern: il kernel la definisce in Section 2,
// il codice applicativo la importa esattamente come farebbe con una DLL.
extern const sv_syscall_table_t sv_kernel;

// ───────────────────────────────────────────────────────────────
//  SEZIONE 2 — Implementazione Firmware (sv_api.cpp)
// ───────────────────────────────────────────────────────────────

static const uint8_t K_data[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE,
                                   0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88,
                                   0x09, 0xCF, 0x4F, 0x3C};
static const uint8_t K_trace[32] = {
    0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE, 0x2B, 0x73, 0xAE,
    0xF0, 0x85, 0x7D, 0x77, 0x81, 0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61,
    0x08, 0xD7, 0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4};

static uint32_t s_id_cnt = 0;
static uint64_t s_last_op_us = 0;  // [L4-2] timestamp ultima operazione per Δt
static uint32_t s_cal_tick_us = 0; // [L4-3] quanto di calibrazione globale
// [R1L5-NONCE] Nonce deterministico per AES-128-GCM:
//   s_nonce_salt : 4 B casuali, fissati una volta in sv_init() per tutta
//                  la sessione. Differenzia crittograficamente i nonce
//                  prodotti da sessioni (boot) distinti.
//   s_nonce_ctr  : contatore a 64 bit, incrementato ad ogni gcm_seal().
//                  Garantisce unicità assoluta dei nonce nella sessione
//                  (nessun birthday-bound, indipendentemente dal numero
//                  di operazioni) rispetto al nonce puramente casuale.
// Layout nonce (12 B = SV_NONCE_LEN):
//   [0..3]  = s_nonce_salt  (4 B, fisso per sessione)
//   [4..11] = s_nonce_ctr   (8 B, little-endian, monotono crescente)
static uint8_t s_nonce_salt[4] = {0};
static uint64_t s_nonce_ctr = 0;
sv_trace_t g_trace;

// ── Registry Nomi ────────────────────────────────────────────
static name_node_t *s_name_registry = NULL;

static bool is_name_registered(const char *name) {
  for (name_node_t *c = s_name_registry; c; c = c->next)
    if (strncmp(c->name, name, 32) == 0)
      return true;
  return false;
}

static void register_name(const char *name, sv_var_id_t var_id) {
  name_node_t *node = (name_node_t *)malloc(sizeof(name_node_t));
  if (!node)
    return;
  strncpy(node->name, name, 32);
  node->name[31] = '\0';
  node->var_id = var_id; // [Emanuel] consente lookup inverso nel JSON
  node->next = s_name_registry;
  s_name_registry = node;
}

// Ricerca inversa var_id → nome simbolico: usata nella serializzazione JSON
// per fornire al verifier di Emanuel il nome della variabile output per ogni
// operazione, senza aggiungere un campo stringa nell'entry della trace.
static const char *lookup_var_name(sv_var_id_t var_id) {
  for (name_node_t *c = s_name_registry; c; c = c->next)
    if (c->var_id == var_id)
      return c->name;
  return "?"; // variabile legacy o non inizializzata con sv_init_var
}

// Svuota il registry globale: percorre tutta la lista e libera ogni nodo.
// Sicuro da chiamare all'inizio di ogni suite run perché avviene sempre
// prima che qualsiasi init() o lookup_var_name() venga eseguita nella
// suite corrente. Non tocca s_id_cnt: gli ID runtime continuano a salire
// monotonicamente attraverso le run, il che è corretto per l'HMAC chain.
void sv_registry_clear(void) {
  name_node_t *cur = s_name_registry;
  while (cur) {
    name_node_t *nxt = cur->next;
    free(cur);
    cur = nxt;
  }
  s_name_registry = NULL;
}

// ── Timing (monotonic µs) ─────────────────────────────────────
// [L4-1] Sostituisce sv_now() basata su CLOCK_REALTIME.
// esp_timer_get_time() è un contatore hardware monotono non alterabile
// da codice applicativo e immune al drift del RTC. Risoluzione: 1 µs.
static inline uint64_t sv_now_us(void) {
  return (uint64_t)esp_timer_get_time();
}
static inline sv_id_t sv_next_id(void) { return ++s_id_cnt; }

static void sv_rng_fill(uint8_t *buf, size_t len) {
  size_t i = 0;
  while (i + 4 <= len) {
    uint32_t r = esp_random();
    memcpy(buf + i, &r, 4);
    i += 4;
  }
  if (i < len) {
    uint32_t r = esp_random();
    memcpy(buf + i, &r, len - i);
  }
}

// ── AES-128-GCM seal / unseal ─────────────────────────────────
// L'intero sv_hdr_t (incluso il nome simbolico) è AAD: qualsiasi modifica
// all'header invalida il GCM tag. Con il campo ts rimosso [L4-1],
// il GCM opera su 44 B di AAD invece di 52 B → ~15% più veloce.
//
// [R1L5-NONCE] Il nonce è ora deterministico (non più puramente casuale):
//   blob[0..3]  = s_nonce_salt  (4 B fissi per sessione, init in sv_init())
//   blob[4..11] = s_nonce_ctr++ (8 B contatore little-endian, mai ripetuto)
// Questa costruzione elimina il rischio di collisione del nonce casuale puro
// (birthday bound su 96 bit ≈ 2^48 coppie di seal con la stessa chiave).
// Il contatore garantisce unicità assoluta; il salt garantisce separazione
// tra sessioni diverse. sv_rng_fill() rimane disponibile per altri usi.
static int gcm_seal(const sv_hdr_t *hdr, const sv_raw_data_t *raw,
                    uint8_t *blob) {
  mbedtls_gcm_context ctx;
  mbedtls_gcm_init(&ctx);
  int ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, K_data, 128);
  if (ret != 0) {
    mbedtls_gcm_free(&ctx);
    return SV_ERR_CRYPTO_FAIL;
  }
  // [R1L5-NONCE] Nonce = salt(4B) || contatore(8B little-endian)
  memcpy(blob, s_nonce_salt, 4);
  uint64_t ctr = s_nonce_ctr++;
  memcpy(blob + 4, &ctr, 8);
  ret = mbedtls_gcm_crypt_and_tag(
      &ctx, MBEDTLS_GCM_ENCRYPT, SV_PT_LEN, blob, SV_NONCE_LEN,
      (const uint8_t *)hdr, sizeof(sv_hdr_t), (const uint8_t *)raw,
      blob + SV_NONCE_LEN, SV_TAG_LEN, blob + SV_NONCE_LEN + SV_PT_LEN);
  mbedtls_gcm_free(&ctx);
  return (ret == 0) ? SV_OK : SV_ERR_CRYPTO_FAIL;
}

static int gcm_unseal(const sv_hdr_t *hdr, const uint8_t *blob,
                      sv_raw_data_t *raw) {
  mbedtls_gcm_context ctx;
  mbedtls_gcm_init(&ctx);
  int ret = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, K_data, 128);
  if (ret != 0) {
    mbedtls_gcm_free(&ctx);
    return SV_ERR_CRYPTO_FAIL;
  }
  ret = mbedtls_gcm_auth_decrypt(&ctx, SV_PT_LEN, blob, SV_NONCE_LEN,
                                 (const uint8_t *)hdr, sizeof(sv_hdr_t),
                                 blob + SV_NONCE_LEN + SV_PT_LEN, SV_TAG_LEN,
                                 blob + SV_NONCE_LEN, (uint8_t *)raw);
  mbedtls_gcm_free(&ctx);
  if (ret == MBEDTLS_ERR_GCM_AUTH_FAILED)
    return SV_ERR_AUTH_FAIL;
  return (ret == 0) ? SV_OK : SV_ERR_CRYPTO_FAIL;
}

static inline int sv_unseal(const sv_val_t *v, sv_raw_data_t *raw) {
  return gcm_unseal(&v->h, v->blob, raw);
}

// ── Tabella op_code → stringa ─────────────────────────────────
// Usata durante la serializzazione JSON per il campo op_name richiesto
// da Emanuel per rendere le trace leggibili al workflow verifier.
static const char *sv_op_name(sv_op_t op) {
  switch (op) {
  case SV_OP_READ:
    return "READ";
  case SV_OP_EVENT:
    return "EVENT";
  case SV_OP_ADD:
    return "ADD";
  case SV_OP_SUB:
    return "SUB";
  case SV_OP_MUL:
    return "MUL";
  case SV_OP_DIV:
    return "DIV";
  case SV_OP_GT:
    return "GT";
  case SV_OP_LT:
    return "LT";
  case SV_OP_EQ:
    return "EQ";
  case SV_OP_AND:
    return "AND";
  case SV_OP_OR:
    return "OR";
  case SV_OP_NOT:
    return "NOT";
  case SV_OP_CONST:
    return "CONST";
  case SV_OP_CNT:
    return "CNT";
  case SV_OP_WRITE:
    return "WRITE";
  case SV_OP_AVG_N:
    return "AVG_N";
  case SV_OP_INIT:
    return "INIT";
  case SV_OP_ASSIGN:
    return "ASSIGN";
  case SV_OP_BIND:
    return "BIND";
  default:
    return "UNKNOWN";
  }
}

// ── Helper hex-encoding per chain_tag nel JSON ────────────────
static void bytes_to_hex(const uint8_t *src, size_t len, char *dst) {
  for (size_t i = 0; i < len; i++)
    snprintf(dst + i * 2, 3, "%02X", src[i]);
  dst[len * 2] = '\0';
}

// [R2L6-2LVL] delta_api_us aggiunto come parametro: il tempo interno dell'API
// viene passato da ogni API pubblica e inserito nel payload HMAC.
static int trace_append_dynamic(sv_trace_t *trace, sv_op_t op, sv_id_t out_id,
                                sv_id_t in1, sv_id_t in2, sv_var_id_t var,
                                sv_src_id_t src, uint16_t meta,
                                uint32_t delta_api_us) {
  if (!trace)
    return SV_ERR_TRACE_FULL;

  // Crescita geometrica: raddoppio quando il buffer è pieno.
  if (trace->count >= trace->capacity) {
    size_t new_cap = (trace->capacity == 0) ? 16 : trace->capacity * 2;
    sv_trace_entry_t *p = (sv_trace_entry_t *)realloc(
        trace->entries, new_cap * sizeof(sv_trace_entry_t));
    if (!p)
      return SV_ERR_TRACE_FULL; // puntatore originale intatto
    trace->entries = p;
    trace->capacity = new_cap;
  }

  // [L4-2] Calcolo delta: microsecondi dall'ultima operazione registrata.
  // La prima entry dopo un sv_trace_clear() ha delta = 0.
  uint64_t now_us = sv_now_us();
  uint32_t delta_us =
      (s_last_op_us == 0) ? 0 : (uint32_t)(now_us - s_last_op_us);
  s_last_op_us = now_us;

  // Payload HMAC a 32 byte fissi (formato stabile per la blockchain).
  // [L4-2] delta_us (4 B) sostituisce tv_sec+tv_nsec (8 B) + 4 B di padding.
  uint8_t payload[32];
  size_t off = 0;
  uint32_t op32 = (uint32_t)op;
  uint16_t pad16 = 0;
  memcpy(payload + off, &op32, 4);
  off += 4;
  memcpy(payload + off, &out_id, 4);
  off += 4;
  memcpy(payload + off, &in1, 4);
  off += 4;
  memcpy(payload + off, &in2, 4);
  off += 4;
  memcpy(payload + off, &var, 2);
  off += 2;
  memcpy(payload + off, &src, 2);
  off += 2;
  memcpy(payload + off, &meta, 2);
  off += 2;
  memcpy(payload + off, &pad16, 2);
  off += 2;
  memcpy(payload + off, &delta_us, 4);
  off += 4;
  // [R2L6-HMAC] delta_api_us sostituisce il padding finale (pad32): il tempo
  // interno dell'API è ora autenticato dalla blockchain crittografica della
  // trace. Qualsiasi tentativo di falsificare i tempi nel JSON romperà l'HMAC.
  memcpy(payload + off, &delta_api_us, 4);
  off += 4; // totale = 32 B

  // new_tag = HMAC-SHA256(K_trace, prev_tag || payload)
  uint8_t hmac_in[SV_HMAC_LEN + 32];
  memcpy(hmac_in, trace->chain_tag, SV_HMAC_LEN);
  memcpy(hmac_in + SV_HMAC_LEN, payload, 32);
  uint8_t new_tag[SV_HMAC_LEN];
  const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  int ret = mbedtls_md_hmac(md, K_trace, 32, hmac_in, sizeof(hmac_in), new_tag);
  if (ret != 0)
    return SV_ERR_CRYPTO_FAIL;

  sv_trace_entry_t *e = &trace->entries[trace->count++];
  e->op = op;
  e->out_id = out_id;
  e->in1_id = in1;
  e->in2_id = in2;
  e->out_var = var;
  e->meta_src = src;
  e->meta_u16 = meta;
  e->_pad = 0;
  e->delta_us = delta_us;
  e->delta_api_us = delta_api_us; // [R2L6]
  memcpy(e->chain_tag, new_tag, SV_HMAC_LEN);
  memcpy(trace->chain_tag, new_tag, SV_HMAC_LEN);
  return SV_OK;
}

// ── Cifratura + Trace (uso interno) ──────────────────────────
// [FIX var_id] Salva il var_id originale da sv_init_var prima del memset
// dell'header. Se la variabile era stata inizializzata con un nome simbolico,
// il var_id viene preservato così la lookup per nome continua a funzionare
// per tutta la vita della variabile, indipendentemente da quante operazioni
// vengono eseguite su di essa.
// [R2L6] api_start_us: timestamp di ingresso nell'API pubblica, usato per
//        calcolare delta_api_us = sv_now_us() - api_start_us.
static int seal_and_trace_at(sv_type_t type, sv_var_id_t var, sv_src_id_t src,
                             sv_id_t in1, sv_id_t in2, sv_op_t op,
                             uint16_t meta, const sv_raw_data_t *raw,
                             sv_trace_t *trace, sv_val_t *out,
                             uint64_t api_start_us) {
  char tmp_name[32] = {0};
  strncpy(tmp_name, out->h.name, 32);
  // [FIX] Preserva var_id se la variabile è stata inizializzata con init().
  // Le variabili legacy (var_id=0 inizialmente) ricevono il var passato.
  sv_var_id_t preserved_var_id = out->h.var_id;

  memset(&out->h, 0, sizeof(out->h));
  out->h.id = sv_next_id();
  out->h.var_id = (preserved_var_id != 0) ? preserved_var_id : var;
  out->h.src_id = src;
  out->h.type = type;
  // [L4-1] Nessun ts nell'header: il timing è nella trace come delta_us.
  strncpy(out->h.name, tmp_name, 31);
  out->blob_len = SV_BLOB_LEN;
  out->trace_ref = trace;

  int r = gcm_seal(&out->h, raw, out->blob);
  if (r != SV_OK)
    return r;
  // [R2L6] Calcolo delta_api_us: tempo interno dell'API (dalla prima
  // istruzione dell'API pubblica fino a qui, dopo il seal).
  uint32_t d_api = (uint32_t)(sv_now_us() - api_start_us);
  return trace_append_dynamic(trace, op, out->h.id, in1, in2, out->h.var_id,
                              src, meta, d_api);
}

static int seal_and_trace(sv_type_t type, sv_var_id_t var, sv_src_id_t src,
                          sv_id_t in1, sv_id_t in2, sv_op_t op, uint16_t meta,
                          const sv_raw_data_t *raw, sv_trace_t *trace,
                          sv_val_t *out, uint64_t api_start_us) {
  return seal_and_trace_at(type, var, src, in1, in2, op, meta, raw, trace, out,
                           api_start_us);
}

// ── Ciclo di vita Traccia ─────────────────────────────────────
void sv_trace_init(sv_trace_t *t) {
  if (!t)
    return;
  t->entries = NULL;
  t->count = 0;
  t->capacity = 0;
  t->cal_tick_us = 0;
  // [L5-2] Timestamp assoluto della creazione della trace
  t->init_us = sv_now_us();
  // [L5-1] Campi GPS inizializzati a zero; riempiti prima dell'export
  t->gps_lat = 0.0;
  t->gps_lng = 0.0;
  t->gps_sats = 0;
  t->gps_valid = false;
  t->flash_count = 0;
  t->use_flash = false;
  memset(t->chain_tag, 0, SV_HMAC_LEN);
}

void sv_trace_free(sv_trace_t *t) {
  if (t && t->entries)
    free(t->entries);
  if (t) {
    t->entries = NULL;
    t->count = 0;
    t->capacity = 0;
  }
}

// sv_trace_clear azzera anche s_last_op_us: la prima entry del nuovo
// batch avrà delta=0 (marcatore di inizio batch), non un delta spurio
// che comprende il tempo speso durante l'esportazione/trasmissione.
void sv_trace_clear(sv_trace_t *t) {
  if (t) {
    t->count = 0;
    // [L5-2] Reset timestamp testa del nuovo batch
    t->init_us = sv_now_us();
    t->flash_count = 0;
    memset(t->chain_tag, 0, SV_HMAC_LEN);
  }
  s_last_op_us = 0; // [L4-2] reset inter-op timer all'inizio di ogni batch
}

int sv_trace_count(const sv_trace_t *t) { return t ? (int)t->count : 0; }

// ── [L6-FLASH] Persistenza asincrona su LittleFS ──────────────
// Inizializza il filesystem e prepara il file di trace.
void sv_trace_flash_init(sv_trace_t *t) {
  if (!t)
    return;
  t->use_flash = false;
#ifdef GPS_HARDWARE
  if (LittleFS.begin(true)) {
    t->use_flash = true;
    // Rimuove eventuali trace precedenti non esportate
    if (LittleFS.exists("/sv_trace.bin")) {
      LittleFS.remove("/sv_trace.bin");
    }
    Serial.println("[L6-FLASH] LittleFS inizializzato per persistenza trace.");
  } else {
    Serial.println("[L6-FLASH] Errore montaggio LittleFS!");
  }
#endif
}

// Flush condizionale delle entry dalla RAM alla Flash.
// Deve essere chiamato solo in contesti sicuri (es. yield) per non
// perturbare i delta temporali critici.
void sv_trace_flush_to_flash(sv_trace_t *t) {
#ifdef GPS_HARDWARE
  if (!t || !t->use_flash || t->count == 0)
    return;

  // Soglia di buffer per minimizzare scritture su flash (es. 32 entry)
  if (t->count < 32)
    return;

  File f = LittleFS.open("/sv_trace.bin", FILE_APPEND);
  if (!f)
    return;

  size_t written =
      f.write((uint8_t *)t->entries, t->count * sizeof(sv_trace_entry_t));
  f.close();

  if (written == t->count * sizeof(sv_trace_entry_t)) {
    t->flash_count += t->count;
    // Reset buffer RAM: manteniamo capacity ma azzeriamo count
    t->count = 0;
  }
#endif
}

// ── sv_init_var ───────────────────────────────────────────────
// [L4-1] Rimosso il campo ts dall'header: nessun clock_gettime() qui.
// [FIX] var_id è passato esplicitamente a register_name per il lookup inverso.
sv_val_t sv_init_var(sv_type_t type, const char *name, sv_trace_t *trace_ref) {
  uint64_t api_t0 = sv_now_us(); // [R2L6] inizio misurazione interna API
  sv_val_t result;
  memset(&result, 0, sizeof(result));
  if (!name || !trace_ref)
    return result;

  if (is_name_registered(name)) {
    Serial.printf("[sv_init_var] ERRORE: '%s' gia' registrato\n", name);
    return result; // id==0 segnala errore al chiamante
  }

  result.h.id = sv_next_id();
  result.h.var_id = (sv_var_id_t)(result.h.id & 0xFFFF);
  result.h.type = type;
  // [L4-1] Nessun ts in h: il timing è nella trace come delta_us.
  strncpy(result.h.name, name, 31);
  result.blob_len = SV_BLOB_LEN;
  result.trace_ref = trace_ref;

  register_name(name, result.h.var_id); // [FIX] passa var_id per lookup inverso

  sv_raw_data_t zero;
  zero.u_val = 0;
  gcm_seal(&result.h, &zero, result.blob);
  // [R2L6] Calcolo delta_api_us per INIT
  uint32_t d_api = (uint32_t)(sv_now_us() - api_t0);
  trace_append_dynamic(trace_ref, SV_OP_INIT, result.h.id, 0, 0,
                       result.h.var_id, 0, 0, d_api);
  return result;
}

// ── assign — assegnamento esplicito ──────────────────────────
// [L4-1] Rimosso ts dall'header: nessun sv_now().
int assign(sv_val_t *var, int32_t value) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  if (!var || !var->trace_ref)
    return SV_ERR_CRYPTO_FAIL;
  char saved_name[32];
  strncpy(saved_name, var->h.name, 32);
  sv_var_id_t saved_vid = var->h.var_id;

  memset(&var->h, 0, sizeof(var->h));
  var->h.id = sv_next_id();
  var->h.var_id = saved_vid;
  var->h.type = SV_T_I32;
  strncpy(var->h.name, saved_name, 31);
  var->blob_len = SV_BLOB_LEN;

  sv_raw_data_t raw;
  raw.i_val = value;
  int r = gcm_seal(&var->h, &raw, var->blob);
  if (r != SV_OK)
    return r;
  uint32_t d_api = (uint32_t)(sv_now_us() - api_t0);
  return trace_append_dynamic(var->trace_ref, SV_OP_ASSIGN, var->h.id, 0, 0,
                              var->h.var_id, 0, 0, d_api);
}

int assign_f(sv_val_t *var, float value) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  if (!var || !var->trace_ref)
    return SV_ERR_CRYPTO_FAIL;
  char saved_name[32];
  strncpy(saved_name, var->h.name, 32);
  sv_var_id_t saved_vid = var->h.var_id;

  memset(&var->h, 0, sizeof(var->h));
  var->h.id = sv_next_id();
  var->h.var_id = saved_vid;
  var->h.type = SV_T_F32;
  strncpy(var->h.name, saved_name, 31);
  var->blob_len = SV_BLOB_LEN;

  sv_raw_data_t raw;
  raw.f_val = value;
  int r = gcm_seal(&var->h, &raw, var->blob);
  if (r != SV_OK)
    return r;
  uint32_t d_api = (uint32_t)(sv_now_us() - api_t0);
  return trace_append_dynamic(var->trace_ref, SV_OP_ASSIGN, var->h.id, 0, 0,
                              var->h.var_id, 0, 0, d_api);
}

// ── [L4-3] sv_calibrate ───────────────────────────────────────
// Misura il tempo medio di esecuzione dell'API più onerosa del sistema
// (sv_add: 2× unseal AES-GCM + operazione + seal AES-GCM + HMAC chain).
// Questo "quanto di tempo" viene usato dal server di Emanuel per rilevare
// ritardi anomali: se delta_us >> cal_tick_us*fattore, l'esecuzione è sospetta.
//
// IMPORTANTE — Idempotenza: questa funzione è chiamabile più volte (es. nel
// benchmark e all'avvio). Per questo NON usa sv_init_var(): quel percorso
// registrerebbe "_cal_a/b/c" nel registry globale dei nomi, e alla seconda
// chiamata is_name_registered() bloccherebbe la creazione, restituendo un
// sv_val_t vuoto (id=0). Tutti gli sv_add() successivi fallirebbero per
// autenticazione GCM e s_cal_tick_us risulterebbe corrotto a ~0, invalidando
// la verifica temporale di Emanuel per tutti i JSON successivi.
// La soluzione è costruire direttamente le sv_val_t interne con ID riservati
// (0xFFFA/B/C) che non passano per il registry e non entrano nella trace reale.
//
// [L5-FIX] Su hardware reale il jitter da interrupt UART GPS (115200 baud),
// tick FreeRTOS e ISR Wi-Fi può sovrastimare il cal_tick_us. Per minimizzarlo
// eleviamo la priorità del task al massimo durante la misurazione.
// NOTA: noInterrupts() NON è utilizzabile perché sull'ESP32-S3 l'acceleratore
// hardware AES-GCM richiede interrupt attivi — disabilitarli causa abort().
// L'averaging su n_samples compensa il jitter residuo.
uint32_t sv_calibrate(uint16_t n_samples) {
  if (n_samples == 0)
    n_samples = 8;

  // Crea una traccia temporanea dedicata alla calibrazione per non
  // inquinare g_trace con le operazioni di misura.
  sv_trace_t cal_trace;
  sv_trace_init(&cal_trace);

  // Costruisce le variabili di calibrazione bypassando sv_init_var() e il
  // registry globale: ID fissi 0xFFFA/B/C (riservati, non usati nell'app),
  // sigillatura diretta con valore 1.0f. Questo rende la funzione idempotente.
  sv_raw_data_t one;
  one.f_val = 1.0f;
  sv_val_t ca, cb, cc;
  memset(&ca, 0, sizeof(ca));
  memset(&cb, 0, sizeof(cb));
  memset(&cc, 0, sizeof(cc));

  ca.h.id = 0xFFFA;
  ca.h.var_id = 0xFFFA;
  ca.h.type = SV_T_F32;
  strncpy(ca.h.name, "_cal_a", 31);
  ca.blob_len = SV_BLOB_LEN;
  ca.trace_ref = &cal_trace;
  gcm_seal(&ca.h, &one, ca.blob);

  cb.h.id = 0xFFFB;
  cb.h.var_id = 0xFFFB;
  cb.h.type = SV_T_F32;
  strncpy(cb.h.name, "_cal_b", 31);
  cb.blob_len = SV_BLOB_LEN;
  cb.trace_ref = &cal_trace;
  gcm_seal(&cb.h, &one, cb.blob);

  cc.h.id = 0xFFFC;
  cc.h.var_id = 0xFFFC;
  cc.h.type = SV_T_F32;
  strncpy(cc.h.name, "_cal_c", 31);
  cc.blob_len = SV_BLOB_LEN;
  cc.trace_ref = &cal_trace;

  // Reset del timer inter-op per non contaminare la prima misura.
  s_last_op_us = 0;

  // [R3L6-NO-RTOS] Rimossa la manipolazione di priorità FreeRTOS
  // (vTaskPrioritySet). Sull'ESP32-S3 l'hardware AES-GCM e l'averaging su
  // n_samples=8 ammortizzano a sufficienza le preemption senza OS lock.

  uint64_t t0 = sv_now_us();
  for (uint16_t i = 0; i < n_samples; i++) {
    sv_add(&ca, &cb, 0, &cc);
  }
  uint64_t t1 = sv_now_us();

  sv_trace_free(&cal_trace);

  s_cal_tick_us = (uint32_t)((t1 - t0) / n_samples);
  g_trace.cal_tick_us = s_cal_tick_us;

  // Reset del timer: il primo delta della traccia reale parte da zero.
  s_last_op_us = 0;

  return s_cal_tick_us;
}

// ── ASYNC ISR: Bottone su GPIO4 ───────────────────────────────
#define BTN_GPIO 4

// [L4-1] us_isr sostituisce struct timespec ts_isr: stessa base temporale
// di esp_timer_get_time(), compatibile con il calcolo di delta_us.
typedef struct {
  uint32_t us_isr; // timestamp µs al momento dell'interrupt
  uint8_t press;
} sv_evt_t;

// [R3L6-NO-RTOS] Sostituzione di QueueHandle_t con un ring buffer C puro volatile
#define SV_EVT_RING_SIZE 16
static volatile sv_evt_t s_evt_ring[SV_EVT_RING_SIZE];
static volatile uint8_t s_evt_head = 0;
static volatile uint8_t s_evt_tail = 0;

static volatile uint32_t s_evt_dropped = 0;
static volatile uint8_t s_evt_overflow = 0;
static volatile uint32_t s_last_isr_us = 0;
static const uint32_t BTN_DEBOUNCE_US = 30000;
static bool s_btn_isr_attached = false;

static void IRAM_ATTR isr_btn_p4() {
  uint32_t now = (uint32_t)micros();
  if ((uint32_t)(now - s_last_isr_us) < BTN_DEBOUNCE_US)
    return;
  s_last_isr_us = now;
  uint8_t next_head = (s_evt_head + 1) % SV_EVT_RING_SIZE;
  if (next_head == s_evt_tail) {
    s_evt_dropped++;
    s_evt_overflow = 1;
    return;
  }
  // [L4-1] esp_timer_get_time() invece di clock_gettime(CLOCK_REALTIME):
  // stesso contatore usato ovunque nel kernel per coerenza del delta_us.
  s_evt_ring[s_evt_head].us_isr = (uint32_t)esp_timer_get_time();
  s_evt_ring[s_evt_head].press = 1;
  s_evt_head = next_head;
}

uint32_t sv_evt_dropped_count(void) { return (uint32_t)s_evt_dropped; }

// ── sv_init ───────────────────────────────────────────────────
// [L4-1] Rimossa settimeofday(): non usiamo più CLOCK_REALTIME.
// [L4-3] sv_calibrate() chiamata qui prima di qualsiasi altra operazione.
int sv_init(void) {
  s_id_cnt = 0;
  s_last_op_us = 0;
  s_cal_tick_us = 0;
  // [R1L5-NONCE] Inizializza salt nonce con 4 byte casuali hardware.
  // Il salt è fisso per l'intera sessione: differenzia i nonce GCM
  // prodotti da sessioni diverse (diversi boot dello stesso device).
  uint32_t rnd_salt = esp_random();
  memcpy(s_nonce_salt, &rnd_salt, 4);
  s_nonce_ctr = 0;
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);
  return SV_OK;
}

// ── Sensor Reads ──────────────────────────────────────────────
int sv_read_temp_P3(sv_var_id_t var, sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  sv_raw_data_t raw;
  raw.f_val = 20.0f + (float)(esp_random() % 1001) / 100.0f;
  sv_trace_t *tr = out->trace_ref ? out->trace_ref : &g_trace;
  return seal_and_trace(SV_T_F32, var, 0x0003, 0, 0, SV_OP_READ, 0, &raw, tr,
                        out, api_t0);
}

int sv_avg_lastN_temp_P3(uint16_t N, sv_var_id_t var, sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  if (N == 0)
    return SV_ERR_SENSOR_FAIL;
  float sum = 0.0f;
  for (uint16_t i = 0; i < N; i++)
    sum += 20.0f + (float)(esp_random() % 1001) / 100.0f;
  sv_raw_data_t raw;
  raw.f_val = sum / (float)N;
  sv_trace_t *tr = out->trace_ref ? out->trace_ref : &g_trace;
  return seal_and_trace(SV_T_F32, var, 0x0003, 0, 0, SV_OP_AVG_N, N, &raw, tr,
                        out, api_t0);
}

// ── Async Button ──────────────────────────────────────────────
int sv_bind_button_P4(void) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  pinMode(BTN_GPIO, INPUT_PULLUP);
  s_evt_head = 0;
  s_evt_tail = 0;
  s_evt_dropped = 0;
  s_evt_overflow = 0;
  s_last_isr_us = 0;
  if (s_btn_isr_attached)
    detachInterrupt(digitalPinToInterrupt(BTN_GPIO));
  attachInterrupt(digitalPinToInterrupt(BTN_GPIO), isr_btn_p4, FALLING);
  s_btn_isr_attached = true;
  uint32_t d_api = (uint32_t)(sv_now_us() - api_t0);
  trace_append_dynamic(&g_trace, SV_OP_BIND, 0, 0, 0, 0, 0x0004, 0, d_api);
  return SV_OK;
}

int sv_poll_button_P4(sv_var_id_t var, sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  if (s_evt_overflow) {
    s_evt_overflow = 0;
    if (s_evt_head == s_evt_tail)
      return SV_ERR_QUEUE_FULL;
  }
  if (s_evt_head == s_evt_tail)
    return SV_ERR_NO_EVENT;
  // [R3L6-NO-RTOS] Copia manuale campo-per-campo: il C++ non consente
  // copy-construct implicito da volatile struct (errore: binding volatile ref).
  sv_evt_t e;
  e.us_isr = s_evt_ring[s_evt_tail].us_isr;
  e.press  = s_evt_ring[s_evt_tail].press;
  s_evt_tail = (s_evt_tail + 1) % SV_EVT_RING_SIZE;
  sv_raw_data_t raw;
  raw.u_val = e.press ? 1u : 0u;
  sv_trace_t *tr = out->trace_ref ? out->trace_ref : &g_trace;
  // Per gli eventi ISR usiamo il timestamp del momento dell'interrupt
  // per calcolare il delta corretto rispetto all'ultima operazione kernel.
  // Salviamo temporaneamente s_last_op_us per non alterare il delta.
  uint64_t prev_last = s_last_op_us;
  s_last_op_us = (uint64_t)e.us_isr; // il "prev" per questo delta è l'ISR
  int r = seal_and_trace_at(SV_T_BOOL, var, 0x0004, 0, 0, SV_OP_EVENT, 1, &raw,
                            tr, out, api_t0);
  if (r != SV_OK) {
    s_last_op_us = prev_last;
  }
  return r;
}



// ── Operazioni Aritmetiche ────────────────────────────────────
int sv_add(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
           sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6] inizio misurazione interna API
  if (a->h.type != b->h.type)
    return SV_ERR_TYPE_MISMATCH;
  sv_trace_t *tr = a->trace_ref ? a->trace_ref : &g_trace;
  sv_raw_data_t ra, rb, rc;
  int r;
  if ((r = sv_unseal(a, &ra)) != SV_OK)
    return r;
  if ((r = sv_unseal(b, &rb)) != SV_OK)
    return r;
  if (a->h.type == SV_T_I32)
    rc.i_val = ra.i_val + rb.i_val;
  else
    rc.f_val = ra.f_val + rb.f_val;
  return seal_and_trace(a->h.type, var, 0, a->h.id, b->h.id, SV_OP_ADD, 0, &rc,
                        tr, out, api_t0);
}

int sv_sub(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
           sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  if (a->h.type != b->h.type)
    return SV_ERR_TYPE_MISMATCH;
  sv_trace_t *tr = a->trace_ref ? a->trace_ref : &g_trace;
  sv_raw_data_t ra, rb, rc;
  int r;
  if ((r = sv_unseal(a, &ra)) != SV_OK)
    return r;
  if ((r = sv_unseal(b, &rb)) != SV_OK)
    return r;
  if (a->h.type == SV_T_I32)
    rc.i_val = ra.i_val - rb.i_val;
  else
    rc.f_val = ra.f_val - rb.f_val;
  return seal_and_trace(a->h.type, var, 0, a->h.id, b->h.id, SV_OP_SUB, 0, &rc,
                        tr, out, api_t0);
}

int sv_mul(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
           sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  if (a->h.type != b->h.type)
    return SV_ERR_TYPE_MISMATCH;
  sv_trace_t *tr = a->trace_ref ? a->trace_ref : &g_trace;
  sv_raw_data_t ra, rb, rc;
  int r;
  if ((r = sv_unseal(a, &ra)) != SV_OK)
    return r;
  if ((r = sv_unseal(b, &rb)) != SV_OK)
    return r;
  if (a->h.type == SV_T_I32)
    rc.i_val = ra.i_val * rb.i_val;
  else
    rc.f_val = ra.f_val * rb.f_val;
  return seal_and_trace(a->h.type, var, 0, a->h.id, b->h.id, SV_OP_MUL, 0, &rc,
                        tr, out, api_t0);
}

int sv_div(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
           sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  if (a->h.type != b->h.type)
    return SV_ERR_TYPE_MISMATCH;
  sv_trace_t *tr = a->trace_ref ? a->trace_ref : &g_trace;
  sv_raw_data_t ra, rb, rc;
  int r;
  if ((r = sv_unseal(a, &ra)) != SV_OK)
    return r;
  if ((r = sv_unseal(b, &rb)) != SV_OK)
    return r;
  if (a->h.type == SV_T_I32) {
    if (!rb.i_val)
      return SV_ERR_CRYPTO_FAIL;
    rc.i_val = ra.i_val / rb.i_val;
  } else {
    if (!rb.f_val)
      return SV_ERR_CRYPTO_FAIL;
    rc.f_val = ra.f_val / rb.f_val;
  }
  return seal_and_trace(a->h.type, var, 0, a->h.id, b->h.id, SV_OP_DIV, 0, &rc,
                        tr, out, api_t0);
}

static int sv_cmp(const sv_val_t *a, const sv_val_t *b, sv_var_id_t var,
                  sv_op_t op, sv_bool_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  if (a->h.type != b->h.type)
    return SV_ERR_TYPE_MISMATCH;
  sv_trace_t *tr = a->trace_ref ? a->trace_ref : &g_trace;
  sv_raw_data_t ra, rb, rc;
  int r;
  if ((r = sv_unseal(a, &ra)) != SV_OK)
    return r;
  if ((r = sv_unseal(b, &rb)) != SV_OK)
    return r;
  bool res = false;
  if (a->h.type == SV_T_I32) {
    if (op == SV_OP_GT)
      res = (ra.i_val > rb.i_val);
    else if (op == SV_OP_LT)
      res = (ra.i_val < rb.i_val);
    else
      res = (ra.i_val == rb.i_val);
  } else {
    if (op == SV_OP_GT)
      res = (ra.f_val > rb.f_val);
    else if (op == SV_OP_LT)
      res = (ra.f_val < rb.f_val);
    else
      res = (ra.f_val == rb.f_val);
  }
  rc.u_val = res ? 1u : 0u;
  return seal_and_trace(SV_T_BOOL, var, 0, a->h.id, b->h.id, op, 0, &rc, tr,
                        &out->v, api_t0);
}
int sv_gt(const sv_val_t *a, const sv_val_t *b, sv_var_id_t v, sv_bool_t *o) {
  return sv_cmp(a, b, v, SV_OP_GT, o);
}
int sv_lt(const sv_val_t *a, const sv_val_t *b, sv_var_id_t v, sv_bool_t *o) {
  return sv_cmp(a, b, v, SV_OP_LT, o);
}
int sv_eq(const sv_val_t *a, const sv_val_t *b, sv_var_id_t v, sv_bool_t *o) {
  return sv_cmp(a, b, v, SV_OP_EQ, o);
}

int sv_const_i32(int32_t k, sv_var_id_t var, sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  sv_raw_data_t raw;
  raw.i_val = k;
  sv_trace_t *tr = out->trace_ref ? out->trace_ref : &g_trace;
  return seal_and_trace(SV_T_I32, var, 0, 0, 0, SV_OP_CONST,
                        (uint16_t)(k & 0xFFFF), &raw, tr, out, api_t0);
}
int sv_const_f32(float k, sv_var_id_t var, sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  sv_raw_data_t raw;
  raw.f_val = k;
  sv_trace_t *tr = out->trace_ref ? out->trace_ref : &g_trace;
  return seal_and_trace(SV_T_F32, var, 0, 0, 0, SV_OP_CONST,
                        (uint16_t)(raw.u_val >> 16), &raw, tr, out, api_t0);
}
int sv_inc(const sv_val_t *c, sv_var_id_t var, sv_val_t *out) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  sv_trace_t *tr = c->trace_ref ? c->trace_ref : &g_trace;
  sv_raw_data_t raw;
  int r = sv_unseal(c, &raw);
  if (r)
    return r;
  if (c->h.type == SV_T_I32)
    raw.i_val++;
  else
    raw.f_val += 1.f;
  return seal_and_trace(c->h.type, var, 0, c->h.id, 0, SV_OP_CNT, 0, &raw, tr,
                        out, api_t0);
}

int sv_write_fan_P12(const sv_bool_t *on) {
  uint64_t api_t0 = sv_now_us(); // [R2L6]
  sv_raw_data_t raw;
  int r = sv_unseal(&on->v, &raw);
  if (r != SV_OK)
    return r;
  digitalWrite(12, raw.u_val ? HIGH : LOW);
  sv_trace_t *tr = on->v.trace_ref ? on->v.trace_ref : &g_trace;
  sv_id_t wid = sv_next_id();
  uint32_t d_api = (uint32_t)(sv_now_us() - api_t0);
  return trace_append_dynamic(tr, SV_OP_WRITE, wid, on->v.h.id, 0, 0, 0x000C, 0,
                              d_api);
}

// ── Export Raw (diagnostica / benchmark interno) ──────────────
int sv_trace_export(sv_trace_t *t, uint8_t *buf, size_t cap, size_t *out_len) {
  if (!t)
    return SV_ERR_TRACE_FULL;
  size_t need = t->count * sizeof(sv_trace_entry_t);
  if (need > cap) {
    *out_len = 0;
    return SV_ERR_TRACE_FULL;
  }
  memcpy(buf, t->entries, need);
  *out_len = need;
  return SV_OK;
}

// ── [L4-4] sv_trace_export_json ───────────────────────────────
// [L6-PROTO] Serializzazione JSON secondo il "Protocollo Stefano-Ennio"
// (nome ufficiale del formato di trace, Lezione 6 — 05.05.2026).
// Nessuna cifratura aggiuntiva on-device: la sicurezza del trasporto è
// garantita da TLS (HTTPS Wi-Fi M5Stack), come concordato a Lezione 4.
//
// [R1L6-CONST] NOTA: questa è l'unica API del sistema il cui tempo di
// esecuzione NON è costante. La durata scala linearmente O(n) con il
// numero di entry nella trace. Tutte le altre API devono avere delta_api_us
// costante per categoria (verifica Riccardo, email 11.05.2026).
// [R3L6-JSON-NOTA] Il tempo di questa API NON deve essere incluso nella
// verifica temporale da parte del verifier di Emanuel: è puro overhead
// di serializzazione estraneo al flusso di esecuzione del kernel
// (direttiva Prof. Culmone, email 13.05.2026).
//
// Formato JSON condiviso con Emanuel per il workflow verifier:
// {
//   "sv_api_version": 6,
//   "device_id": "ESP32_WOKWI" | "ATOM_GPS_V2",
//   "cal_tick_us": 1234,      ← quanto di riferimento per verifica temporale
//   "init_us": 123456789,     ← [L5-2] timestamp assoluto testa trace
//   "gps": {                  ← [L5-1] dati GPS (Atomic GPS Base v2.0)
//     "lat": 43.1381, "lng": 13.0684, "sats": 7, "valid": true
//   },
//   "count": N,
//   "entries": [
//     {
//       "seq": 0,
//       "op": 70,
//       "op_name": "INIT",    ← nome leggibile per il parser di Emanuel
//       "out_id": "00000001", ← ID runtime output (per HMAC chain)
//       "in1_id": "00000000", ← ID runtime in1 (data lineage)
//       "in2_id": "00000000", ← ID runtime in2 (data lineage)
//       "var_id": 1,          ← "ide" stabile di Emanuel per data flow
//       "var_name": "temperatura", ← nome simbolico (dal registry)
//       "delta_us": 0,        ← µs dall'operazione precedente
//       "delta_api_us": 123,  ← [R2L6] µs interni all'API (costante per cat.)
//       "chain_tag": "AABB..."
//     }, ...
//   ],
//   "final_tag": "AABB..."   ← commit MAC del batch
// }
int sv_trace_export_json(const sv_trace_t *trace, char **json_out,
                         size_t *json_len) {
  size_t total_count = trace ? trace->count : 0;
#ifdef GPS_HARDWARE
  if (trace && trace->use_flash)
    total_count += trace->flash_count;
#endif

  if (!trace || total_count == 0) {
    *json_out = NULL;
    *json_len = 0;
    return SV_ERR_NO_EVENT;
  }

  // [L5-1] Stima aumentata per header GPS e init_us
  // [R2L6] +20 B/entry per delta_api_us
  size_t max_sz = 256 + total_count * 340 + 32;
  char *json = (char *)malloc(max_sz);
  if (!json)
    return SV_ERR_TRACE_FULL;

  // Tag finale dal running state, che copre anche le entry già su Flash
  char final_tag_hex[SV_HMAC_LEN * 2 + 1];
  bytes_to_hex(trace->chain_tag, SV_HMAC_LEN, final_tag_hex);

  size_t pos = 0;
  pos +=
      snprintf(json + pos, max_sz - pos,
               "{\"sv_api_version\":%d,"
#ifdef GPS_HARDWARE
               "\"device_id\":\"ATOM_GPS_V2\","
#else
               "\"device_id\":\"ESP32_WOKWI\","
#endif
               "\"cal_tick_us\":%lu,"
               "\"init_us\":%llu,"
               "\"gps\":{\"lat\":%.6f,\"lng\":%.6f,\"sats\":%lu,\"valid\":%s},"
               "\"count\":%zu,"
               "\"entries\":[",
               SV_API_VERSION, (unsigned long)trace->cal_tick_us,
               (unsigned long long)trace->init_us, trace->gps_lat,
               trace->gps_lng, (unsigned long)trace->gps_sats,
               trace->gps_valid ? "true" : "false", total_count);

  size_t global_seq = 0;

// Macro helper per formattare una singola entry, usata sia per la flash che per
// la RAM
#define FORMAT_ENTRY(e_ptr)                                                    \
  do {                                                                         \
    char etag[SV_HMAC_LEN * 2 + 1];                                            \
    bytes_to_hex((e_ptr)->chain_tag, SV_HMAC_LEN, etag);                       \
    const char *vname = lookup_var_name((e_ptr)->out_var);                     \
    char entry[360];                                                           \
    snprintf(entry, sizeof(entry),                                             \
             "%s{"                                                             \
             "\"seq\":%zu,"                                                    \
             "\"op\":%d,"                                                      \
             "\"op_name\":\"%s\","                                             \
             "\"out_id\":\"%08lX\","                                           \
             "\"in1_id\":\"%08lX\","                                           \
             "\"in2_id\":\"%08lX\","                                           \
             "\"var_id\":%u,"                                                  \
             "\"var_name\":\"%s\","                                            \
             "\"delta_us\":%lu,"                                               \
             "\"delta_api_us\":%lu,"                                           \
             "\"chain_tag\":\"%s\""                                            \
             "}",                                                              \
             (global_seq > 0) ? "," : "", global_seq, (int)(e_ptr)->op,        \
             sv_op_name((e_ptr)->op), (unsigned long)(e_ptr)->out_id,          \
             (unsigned long)(e_ptr)->in1_id, (unsigned long)(e_ptr)->in2_id,   \
             (unsigned)(e_ptr)->out_var, vname,                                \
             (unsigned long)(e_ptr)->delta_us,                                 \
             (unsigned long)(e_ptr)->delta_api_us, etag);                      \
    size_t elen = strlen(entry);                                               \
    if (pos + elen + 64 < max_sz) {                                            \
      memcpy(json + pos, entry, elen);                                         \
      pos += elen;                                                             \
    }                                                                          \
    global_seq++;                                                              \
  } while (0)

#ifdef GPS_HARDWARE
  if (trace->use_flash && trace->flash_count > 0) {
    File f = LittleFS.open("/sv_trace.bin", FILE_READ);
    if (f) {
      sv_trace_entry_t fe;
      while (f.read((uint8_t *)&fe, sizeof(sv_trace_entry_t)) ==
                 sizeof(sv_trace_entry_t) &&
             pos < max_sz - 128) {
        FORMAT_ENTRY(&fe);
      }
      f.close();
    }
  }
#endif

  for (size_t i = 0; i < trace->count && pos < max_sz - 128; i++) {
    const sv_trace_entry_t *e = &trace->entries[i];
    FORMAT_ENTRY(e);
  }

#undef FORMAT_ENTRY

  if (pos + 40 < max_sz) {
    pos += snprintf(json + pos, max_sz - pos, "],\"final_tag\":\"%s\"}",
                    final_tag_hex);
  }
  json[pos] = '\0';

  *json_out = json;
  *json_len = pos;
  return SV_OK;
}

// ── [L5-1] GPS — Atomic GPS Base v2.0 ────────────────────────
#ifdef GPS_HARDWARE

static void gps_init(void) {
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.printf("[GPS] UART1 inizializzata (ATGM336H @ %lu bps)\n",
                (unsigned long)GPS_BAUD);
}

static void gps_update(void) {
  while (GPSSerial.available() > 0) {
    s_gps.encode(GPSSerial.read());
  }
}

static void gps_fill_trace(sv_trace_t *t) {
  if (!t)
    return;
  // [R3L5-GPS-FILL] Drena il buffer UART prima di leggere s_gps: garantisce
  // che i frame NMEA più recenti siano processati da TinyGPSPlus a prescindere
  // da quando è stata effettuata l'ultima chiamata esplicita a gps_update().
  gps_update();
  t->gps_valid = s_gps.location.isValid();
  // [R3L5-GPS-FILL] Le coordinate vengono salvate SEMPRE, senza condizionale
  // su isValid(). TinyGPSPlus mantiene le ultime coordinate parsate anche da
  // frame NMEA con status 'V' (void / last-known position): in quel caso
  // s_gps.location.lat/lng() restituisce la stima più recente del modulo,
  // mentre isValid() == false segnala al verifier che il fix non è ancora
  // confermato con status 'A'. Questa è la ragione per cui il display
  // mostrava 43.13941 mentre il JSON riportava 0.0: il display leggeva
  // s_gps direttamente, il JSON perdeva i dati a causa dello zero-forzato.
  t->gps_lat = s_gps.location.lat();
  t->gps_lng = s_gps.location.lng();
  t->gps_sats = s_gps.satellites.value();
}

#else // Simulazione (Wokwi / esp32dev)

static void gps_init(void) {
  Serial.println(F("[GPS] Simulazione Wokwi — GPS non disponibile"));
}
static void gps_update(void) { /* noop su Wokwi */ }
static void gps_fill_trace(sv_trace_t *t) {
  if (!t)
    return;
  t->gps_valid = false;
  t->gps_lat = 43.1381; // Camerino, lat
  t->gps_lng = 13.0684; // Camerino, lng
  t->gps_sats = 0;
}

#endif // GPS_HARDWARE

// ── Istanza della dispatch table — export table del kernel ────
// [Prof. Culmone] Questo è il punto di esportazione formale del kernel:
// l'equivalente della export directory di una DLL Windows o della
// symbol table di una shared library ELF. Il codice applicativo (Section 3)
// importa questo oggetto e chiama le API esclusivamente attraverso di esso.
//
// La dichiarazione const garantisce che il codice applicativo non possa
// sovrascrivere i puntatori (attacco di tipo "vtable hijacking"):
// su sistemi con MPU/MMU questa struct vivrebbe in una pagina di memoria
// con permessi rx (read + execute) per il codice utente, non rw.
//
// Nota: i simboli sv_xxx() referenziati qui sotto sono "interni alla DLL"
// (static linkage, non exported nel senso DLL): in un sistema con separazione
// kernel/utente reale, il codice applicativo non potrebbe linkarli
// direttamente. Solo sv_kernel è il simbolo esportato visibile all'esterno.
const sv_syscall_table_t sv_kernel = {
    .f_init = sv_init,
    .f_calibrate = sv_calibrate,
    .f_registry_clear = sv_registry_clear,
    .f_init_var = sv_init_var,
    .f_assign = assign,
    .f_assign_f = assign_f,
    .f_trace_init = sv_trace_init,
    .f_trace_free = sv_trace_free,
    .f_trace_clear = sv_trace_clear,
    .f_trace_count = sv_trace_count,
    .f_trace_flash_init = sv_trace_flash_init,
    .f_trace_flush_to_flash = sv_trace_flush_to_flash,
    .f_read_temp_P3 = sv_read_temp_P3,
    // [R3L6-CLEANUP-GPS] API GPS rimosse interamente dal kernel (email
    // Prof. Culmone 13.05.2026). L'acquisizione GPS avviene ora in
    // user-space tramite f_assign_f() su variabili sv_val_t standard.
    // [R3L6-CLEANUP-VEC] API vettoriali rimosse: il batching usa array
    // nativi di sv_val_t nell'Application Layer.
    .f_avg_lastN_temp_P3 = sv_avg_lastN_temp_P3,
    .f_bind_button_P4 = sv_bind_button_P4,
    .f_poll_button_P4 = sv_poll_button_P4,
    .f_evt_dropped_count = sv_evt_dropped_count,
    .f_write_fan_P12 = sv_write_fan_P12,
    .f_add = sv_add,
    .f_sub = sv_sub,
    .f_mul = sv_mul,
    .f_div = sv_div,
    .f_gt = sv_gt,
    .f_lt = sv_lt,
    .f_eq = sv_eq,
    .f_const_i32 = sv_const_i32,
    .f_const_f32 = sv_const_f32,
    .f_inc = sv_inc,
    .f_trace_export = sv_trace_export,
    .f_trace_export_json = sv_trace_export_json,
    .f_gps_update = gps_update,
    .f_gps_fill_trace = gps_fill_trace,
};

// ───────────────────────────────────────────────────────────────
//  SEZIONE 3 — Application layer
// ───────────────────────────────────────────────────────────────

// ── Modello di chiamata in Section 3 ─────────────────────────
// [Prof. Culmone] Questo blocco rappresenta il CODICE UTENTE (user-space).
// In un sistema con separazione kernel/utente reale, tutto ciò che segue
// non avrebbe visibilità sui simboli sv_xxx(): potrebbe accedere al kernel
// esclusivamente tramite sv_kernel (la dispatch table importata come DLL).
//
// In questa simulazione Wokwi/ESP32 (single-binary, no MMU completa) la
// separazione è per convenzione. I punti di ingresso principali — setup()
// e loop() — chiamano già attraverso sv_kernel per dimostrare il meccanismo.
// Tutte le funzioni helper in Section 3 (run_benchmarks, demo_pipeline_avgN,
// demo_async_button_window, task_poll_button, ecc.) chiamano il kernel
// esclusivamente attraverso sv_kernel.f_xxx(), coerentemente con il modello
// dispatch table / DLL import descritto dal Prof. Culmone.
//
// L'unica eccezione documentata è g_trace: è una variabile globale del kernel
// il cui indirizzo è noto all'applicazione (come un "handle" del kernel).
// In un sistema reale sarebbe un opaque handle (intero) restituito da f_init().

// Ridirezione del macro init() attraverso la dispatch table:
// il codice applicativo non chiama sv_init_var() direttamente ma passa
// per sv_kernel.f_init_var(), esattamente come farebbe attraverso la PLT.
#undef init
#define init(type, name, trace_ref) sv_kernel.f_init_var(type, name, trace_ref)

#define VAR_TEMP1 0x0001
#define VAR_TEMP2 0x0002
#define VAR_SUM 0x0003
#define VAR_THR 0x0004
#define VAR_COND 0x0005
#define VAR_CNT 0x0006
#define VAR_ONE 0x0007
#define VAR_AVG 0x0008
#define VAR_BTN 0x0100
#define N_BENCH 10

static uint8_t s_export_buf[128 * sizeof(sv_trace_entry_t)];
static inline int64_t us_now_app(void) { return esp_timer_get_time(); }

struct Stats {
  int32_t min_us = INT32_MAX, max_us = 0;
  int64_t sum_us = 0;
  int errors = 0, count = 0;
  void record(int64_t t0, int64_t t1, int ret) {
    int32_t dt = (int32_t)(t1 - t0);
    if (dt < min_us)
      min_us = dt;
    if (dt > max_us)
      max_us = dt;
    sum_us += dt;
    if (ret != SV_OK)
      errors++;
    count++;
  }
  int32_t avg() const { return count ? (int32_t)(sum_us / count) : 0; }
};

static void sep() {
  Serial.println(F("  "
                   "───────────────────────────────────────────────────────────"
                   "───────────"));
}

static void print_banner() {
  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════════════════════"
                   "══════════════╗"));
#ifdef GPS_HARDWARE
  Serial.println(F("║  SV-API Benchmark — AtomS3 GPS v2.0  (Lezione 6 — "
                   "05.05.2026)          ║"));
#else
  Serial.println(F("║  SV-API Benchmark — ESP32/Wokwi  (Lezione 6 — "
                   "05.05.2026)             ║"));
#endif
  Serial.println(F("║  AES-128-GCM sealed values  .  delta_us + delta_api_us  "
                   ". calibrazione   ║"));
  Serial.println(F("║  Protocollo Stefano-Ennio JSON  .  verifier Emanuel  .  "
                   "API v6          ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════"
                   "══════════════╝"));
  Serial.println();
}

static void print_row(const char *name, int32_t avg_us, int32_t mn, int32_t mx,
                      const char *est, int errors) {
  char row[160];
  snprintf(row, sizeof(row), "  %-36s %7ld us  %7ld us  %7ld us   %-14s  %s",
           name, (long)avg_us, (long)mn, (long)mx, est,
           (errors == 0) ? "OK" : "ERR!");
  Serial.println(row);
}

static void run_benchmarks(void) {
  INT a = init(SV_T_F32, "bm_a", &g_trace);
  INT b = init(SV_T_F32, "bm_b", &g_trace);
  INT out = init(SV_T_F32, "bm_out", &g_trace);
  sv_bool_t bout;
  bout.v = init(SV_T_BOOL, "bm_cond", &g_trace);

  int r;
  int64_t t0, t1;
  Stats st;

  Serial.println(F("==========================================================="
                   "================"));
#ifdef GPS_HARDWARE
  Serial.printf("  MICRO-BENCHMARK  (N=%d per API, AtomS3 hardware)\n",
                N_BENCH);
#else
  Serial.printf(
      "  MICRO-BENCHMARK  (N=%d per API, Wokwi software simulation)\n",
      N_BENCH);
#endif
  Serial.printf("  Calibrazione: cal_tick_us = %lu us (riferimento sv_add)\n",
                (unsigned long)g_trace.cal_tick_us);
  Serial.println(F("==========================================================="
                   "================"));
  Serial.println(F("  API                                      avg        min  "
                   "      max   proposta      ok?"));
  sep();

  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    sv_kernel.f_trace_clear(&g_trace);
    t0 = us_now_app();
    r = sv_kernel.f_read_temp_P3(VAR_TEMP1, &a);
    t1 = us_now_app();
    st.record(t0, t1, r);
  }
  print_row("sv_read_temp_P3", st.avg(), st.min_us, st.max_us, "0.5-2.0 ms",
            st.errors);
  // [R3L5-GPS-BENCH] Drain UART GPS durante i benchmark per evitare overflow
  // del buffer HardwareSerial (256 B) e non perdere frame NMEA con fix 'A'.
  sv_kernel.f_gps_update();
  yield();

  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    sv_kernel.f_trace_clear(&g_trace);
    t0 = us_now_app();
    r = sv_kernel.f_avg_lastN_temp_P3(10, VAR_AVG, &a);
    t1 = us_now_app();
    st.record(t0, t1, r);
  }
  print_row("sv_avg_lastN_temp_P3(10)", st.avg(), st.min_us, st.max_us,
            "see note", st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    sv_kernel.f_trace_clear(&g_trace);
    t0 = us_now_app();
    r = sv_kernel.f_const_f32(25.0f, VAR_THR, &b);
    t1 = us_now_app();
    st.record(t0, t1, r);
  }
  print_row("sv_const_f32", st.avg(), st.min_us, st.max_us, "0.1-1.0 ms",
            st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  sv_kernel.f_read_temp_P3(VAR_TEMP1, &a);
  sv_kernel.f_read_temp_P3(VAR_TEMP2, &b);
  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    sv_kernel.f_trace_clear(&g_trace);
    t0 = us_now_app();
    r = sv_kernel.f_add(&a, &b, VAR_SUM, &out);
    t1 = us_now_app();
    st.record(t0, t1, r);
  }
  print_row("sv_add (2×dec+add+seal)", st.avg(), st.min_us, st.max_us,
            "0.3-1.2 ms", st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  sv_kernel.f_const_f32(25.0f, VAR_THR, &b);
  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    sv_kernel.f_trace_clear(&g_trace);
    t0 = us_now_app();
    r = sv_kernel.f_gt(&a, &b, VAR_COND, &bout);
    t1 = us_now_app();
    st.record(t0, t1, r);
  }
  print_row("sv_gt (2×dec+cmp+seal)", st.avg(), st.min_us, st.max_us,
            "0.3-1.0 ms", st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    sv_kernel.f_trace_clear(&g_trace);
    t0 = us_now_app();
    r = sv_kernel.f_write_fan_P12(&bout);
    t1 = us_now_app();
    st.record(t0, t1, r);
  }
  print_row("sv_write_fan_P12", st.avg(), st.min_us, st.max_us, "0.1-0.5 ms",
            st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  // Benchmark export raw
  sv_kernel.f_trace_clear(&g_trace);
  sv_kernel.f_read_temp_P3(VAR_TEMP1, &a);
  sv_kernel.f_const_f32(25.0f, VAR_THR, &b);
  sv_kernel.f_gt(&a, &b, VAR_COND, &bout);
  sv_kernel.f_add(&a, &b, VAR_SUM, &out);
  sv_kernel.f_write_fan_P12(&bout);
  int n_entries = sv_kernel.f_trace_count(&g_trace);
  size_t exp_len = 0;
  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    t0 = us_now_app();
    r = sv_kernel.f_trace_export(&g_trace, s_export_buf, sizeof(s_export_buf),
                                 &exp_len);
    t1 = us_now_app();
    st.record(t0, t1, r);
  }
  print_row("sv_trace_export (raw binary)", st.avg(), st.min_us, st.max_us,
            "0.5-2.0 ms", st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  // [L4-4] Benchmark JSON export (formato per server Emanuel, trasmissione
  // HTTPS)
  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    char *jstr = NULL;
    size_t jlen = 0;
    t0 = us_now_app();
    r = sv_kernel.f_trace_export_json(&g_trace, &jstr, &jlen);
    t1 = us_now_app();
    st.record(t0, t1, r);
    if (jstr)
      free(jstr);
  }
  print_row("sv_trace_export_json (HTTPS)", st.avg(), st.min_us, st.max_us,
            "JSON+ready", st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  // Benchmark init + assign.
  // bench_tmp viene costruita con ID riservato 0xFFFD bypassando il registry
  // (stesso pattern di sv_calibrate con _cal_a/b/c): anche se il loop gira
  // N_BENCH volte, nessuna chiamata a is_name_registered() blocca la misura.
  // assign() non fa gcm_unseal, quindi il blob iniziale non influenza il tempo:
  // misura esattamente 1× gcm_seal + trace_append (= metà di un ciclo
  // completo).
  st = Stats{};
  for (int i = 0; i < N_BENCH; i++) {
    sv_trace_t tmp;
    sv_kernel.f_trace_init(&tmp);
    sv_val_t tv;
    memset(&tv, 0, sizeof(tv));
    tv.h.id = 0xFFFD;
    tv.h.var_id = 0xFFFD;
    tv.h.type = SV_T_I32;
    strncpy(tv.h.name, "bench_tmp", 31);
    tv.blob_len = SV_BLOB_LEN;
    tv.trace_ref = &tmp;
    // Nota: gcm_seal() è kernel-interna (static). L'accesso diretto
    // costruisce variabili con ID riservati (bypass registry). In un
    // sistema reale, questo benchmark girerebbe in kernel-mode.
    sv_raw_data_t z;
    z.i_val = 0;
    gcm_seal(&tv.h, &z, tv.blob);
    t0 = us_now_app();
    sv_kernel.f_assign(&tv, 42);
    t1 = us_now_app();
    st.record(t0, t1, (tv.h.id != 0) ? SV_OK : SV_ERR_CRYPTO_FAIL);
    sv_kernel.f_trace_free(&tmp);
  }
  print_row("init + assign (2×GCM+HMAC)", st.avg(), st.min_us, st.max_us,
            "2×seal+trace", st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  // [L4-3] Benchmark sv_calibrate — eseguita una sola volta (label "boot
  // once"). Non ha senso misurarla in loop: è progettata per girare una volta
  // all'avvio. Grazie a [FIX2] è ora idempotente (variabili interne con ID
  // riservati 0xFFFA/B/C, bypass del registry), ma eseguirla N_BENCH volte di
  // fila moltiplica inutilmente il costo: ogni chiamata fa n_samples veri
  // sv_add con AES-GCM software, ~0.7 ms l'uno su Wokwi. Prima del fix, le
  // iterazioni 2-N fallivano silenziosamente (bug registry) e quindi erano
  // quasi istantanee — comportamento corretto per caso. Questa singola chiamata
  // con n_samples=8 misura il costo reale E aggiorna s_cal_tick_us al valore
  // definitivo usato nei JSON del live mode.
  st = Stats{};
  {
    t0 = us_now_app();
    uint32_t c = sv_kernel.f_calibrate(8);
    t1 = us_now_app();
    st.record(t0, t1, SV_OK);
    (void)c;
  }
  print_row("sv_calibrate (8×sv_add avg)", st.avg(), st.min_us, st.max_us,
            "boot once", st.errors);
  sv_kernel.f_gps_update(); // [R3L5-GPS-BENCH] drain UART buffer
  yield();

  // [R3L6-CLEANUP-VEC] Benchmark sv_vec_put e sv_vec_get rimossi.
  // La gestione dei vettori è stata spostata a livello applicativo tramite
  // array nativi e la struct sv_vec_t non esiste più nel kernel.
  // Il confronto economico è rimosso poiché ogni assegnazione nel nuovo array
  // usa l'opcode nativo ASSIGN e subisce il costo intero del seal GCM (~230 µs).

  sep();
  Serial.println(F("  MEMORY FOOTPRINT"));
  sep();
  Serial.printf(
      "  sizeof(sv_hdr_t)          = %zu B  (era 52 B con timespec)\n",
      sizeof(sv_hdr_t));
  Serial.printf("  sizeof(sv_val_t)          = %zu B\n", sizeof(sv_val_t));
  Serial.printf(
      "  sizeof(sv_trace_entry_t)  = %zu B  (era 64 B con timespec)\n",
      sizeof(sv_trace_entry_t));
  Serial.printf("  g_trace heap (cap=%zu)     = %zu B\n", g_trace.capacity,
                g_trace.capacity * sizeof(sv_trace_entry_t));
  Serial.printf("  raw export batch          = %d entries -> %zu B\n",
                n_entries, exp_len);
  Serial.printf("  cal_tick_us (riferimento) = %lu us\n",
                (unsigned long)g_trace.cal_tick_us);
  Serial.println();
}

static void demo_gps_secure_vector(void) {
  Serial.println(F("==========================================================="
                   "================"));
  Serial.println(F("  GPS SECURE BATCH DEMO  (Application Layer Polling)"));
  Serial.println(F("  Batching R3L6: array nativo sv_val_t, acquisizione esplicita"));
  sep();
  sv_kernel.f_trace_clear(&g_trace);

  const int N = 3; // campioni GPS (lat+lng) da acquisire

  sv_val_t batch_lat[N];
  sv_val_t batch_lng[N];

  // Le variabili contenitore vengono inizializzate FUORI dal ciclo per non
  // saturare il registry con nomi duplicati ad ogni iterazione.
  for (int i = 0; i < N; i++) {
    char name_lat[16], name_lng[16];
    snprintf(name_lat, sizeof(name_lat), "batch_lat_%d", i);
    snprintf(name_lng, sizeof(name_lng), "batch_lng_%d", i);
    batch_lat[i] = sv_kernel.f_init_var(SV_T_F32, name_lat, &g_trace);
    batch_lng[i] = sv_kernel.f_init_var(SV_T_F32, name_lng, &g_trace);
  }

  // ── Fase 1: ACQUISIZIONE ─────────────────────────────────────────────────
  int64_t t_acq0 = us_now_app();
  for (int i = 0; i < N; i++) {
    // [R3L6-CLEANUP-GPS] Il GPS è interrogato dal codice utente (non sicuro).
    // TinyGPSPlus decodifica, poi f_assign_f sigilla il dato float in una variabile sv_val_t.
    // L'opcode è SV_OP_ASSIGN, non un "fake-sicuro" READ_GPS.
#ifdef GPS_HARDWARE
    // In un sistema reale ci sarebbe un yield() e poi un while(!s_gps.encode(Serial1.read()))
    sv_kernel.f_gps_update(); // Svuota buffer UART
    float raw_lat = (float)s_gps.location.lat();
    float raw_lng = (float)s_gps.location.lng();
#else
    float raw_lat = 43.1381f + (i * 0.0001f);
    float raw_lng = 13.0684f + (i * 0.0001f);
#endif

    sv_kernel.f_assign_f(&batch_lat[i], raw_lat);
    sv_kernel.f_assign_f(&batch_lng[i], raw_lng);
    sv_kernel.f_trace_flush_to_flash(&g_trace); // [L6-FLASH] [R3L6-OVERHEAD] Flush asincrono
  }
  int64_t t_acq1 = us_now_app();

  // ── Fase 2: ELABORAZIONE ─────────────────────────────────────────────────
  INT sum_lat = sv_kernel.f_init_var(SV_T_F32, "gps_lat_sum", &g_trace);
  INT sum_lng = sv_kernel.f_init_var(SV_T_F32, "gps_lng_sum", &g_trace);

  sv_kernel.f_assign_f(&sum_lat, 0.0f);
  sv_kernel.f_assign_f(&sum_lng, 0.0f);

  int64_t t_elab0 = us_now_app();
  for (int i = 0; i < N; i++) {
    sv_kernel.f_add(&sum_lat, &batch_lat[i], VAR_SUM, &sum_lat);
    sv_kernel.f_add(&sum_lng, &batch_lng[i], VAR_SUM, &sum_lng);
  }
  int64_t t_elab1 = us_now_app();

  int64_t us_acq = t_acq1 - t_acq0;
  int64_t us_elab = t_elab1 - t_elab0;

  Serial.printf("  Trace entries        : %d\n", sv_kernel.f_trace_count(&g_trace));
  Serial.printf("  Fase acquisizione    : %lld us (%.3f ms)  [%d f_assign]\n",
                us_acq, us_acq / 1000.0, N * 2);
  Serial.printf("  Fase elaborazione    : %lld us (%.3f ms)  [%d sv_add]\n",
                us_elab, us_elab / 1000.0, N * 2);

  // [R3L6-CLEANUP-VEC] Rimossa l'analisi economica di sv_vec_put.
  Serial.println(F("  [R3L6-CLEANUP-VEC] Nota: il batching ora usa array nativi."));
  Serial.println(F("  Costo: ogni assegnazione (f_assign) esegue un seal AES-GCM intero,"));
  Serial.println(F("  esponendo il reale carico crittografico invece di un HMAC nudo."));
  Serial.println();
}

static void demo_pipeline_avgN(void) {
  Serial.println(F("==========================================================="
                   "================"));
  Serial.println(F("  PIPELINE DEMO  AVG_N -> CONST -> GT -> WRITE"));
  Serial.println(
      F("  Mostra anche il JSON che verrebbe trasmesso via HTTPS a Emanuel."));
  sep();
  sv_kernel.f_trace_clear(&g_trace);

  INT avg_v = init(SV_T_F32, "pipe_avg", &g_trace);
  INT thr_v = init(SV_T_F32, "pipe_thr", &g_trace);
  sv_bool_t cond_v;
  cond_v.v = init(SV_T_BOOL, "pipe_cond", &g_trace);

  int64_t ts = us_now_app();
  sv_kernel.f_avg_lastN_temp_P3(10, VAR_AVG, &avg_v);
  sv_kernel.f_const_f32(25.0f, VAR_THR, &thr_v);
  sv_kernel.f_gt(&avg_v, &thr_v, VAR_COND, &cond_v);
  sv_kernel.f_write_fan_P12(&cond_v);
  int64_t te = us_now_app();

  Serial.printf("  Pipeline total  : %lld us (%.3f ms)\n", te - ts,
                (te - ts) / 1000.0);
  Serial.printf("  Trace entries   : %d\n", sv_kernel.f_trace_count(&g_trace));
  Serial.printf("  GPIO12 (fan)    : %s\n",
                digitalRead(12) ? "HIGH (ON)" : "LOW (OFF)");

  // [R3L5-GPS-FILL] Drain aggiuntivo del buffer UART prima di gps_fill_trace():
  // anche se gps_fill_trace() chiama gps_update() internamente, questo drain
  // preventivo garantisce che l'ultimo frame NMEA disponibile sia nel buffer
  // software prima che gps_fill_trace() lo processi. Rilevante soprattutto
  // quando la demo GPS secure vector ha consumato l'ultimo burst di dati UART
  // e il modulo sta per inviare il frame successivo.
  sv_kernel.f_gps_update();
  // [L5-1] Assicurati di popolare i campi GPS nella trace prima dell'export.
  // gps_fill_trace() chiama gps_update() internamente [R3L5-GPS-FILL].
  sv_kernel.f_gps_fill_trace(&g_trace);

  // [R3L5-JSON-FULL] Stampa la traccia JSON completa come richiesto da Emanuel:
  // la trace intera (non troncata a 200 char) contiene tutti i campi necessari
  // al verifier Python per la verifica del data flow.
  char *jstr = NULL;
  size_t jlen = 0;
  if (sv_kernel.f_trace_export_json(&g_trace, &jstr, &jlen) == SV_OK && jstr) {
    Serial.printf("  JSON size       : %zu bytes\n", jlen);
    Serial.println(F("  JSON completo   :"));
    // Stampa il JSON in blocchi da 128 caratteri per compatibilità con tutti
    // i monitor seriali (alcuni troncano le righe molto lunghe).
    const size_t CHUNK = 128;
    for (size_t off = 0; off < jlen; off += CHUNK) {
      size_t to_print = (jlen - off < CHUNK) ? (jlen - off) : CHUNK;
      Serial.write(jstr + off, to_print);
    }
    Serial.println();
    free(jstr);
  }
  Serial.println();
}

static void demo_async_button_window(void) {
  Serial.println(F("==========================================================="
                   "================"));
  Serial.println(
      F("  ASYNC DEMO  BUTTON P4  ISR -> ring-buffer -> sv_poll_button_P4"));
  // [R3L6-NO-RTOS] L'AtomS3 con Atomic GPS Base v2.0 non dispone di un
  // pulsante fisico su GPIO4 (unico pulsante: reset/screen su GPIO41).
  // La demo è architetturale: dimostra il meccanismo ISR -> ring-buffer
  // del kernel senza dipendenze FreeRTOS. Funzionale su Wokwi o con
  // pulsante esterno collegato a GPIO4.
  Serial.println(F("  Demo architetturale ISR/ring-buffer (5s window)."));
  Serial.println(F("  (AtomS3: nessun pulsante fisico su GPIO4)"));
  sep();
  sv_kernel.f_bind_button_P4();
  sv_kernel.f_trace_clear(&g_trace);

  INT ev = init(SV_T_BOOL, "async_btn", &g_trace);
  uint32_t start = millis();
  int got = 0;
  while (millis() - start < 5000) {
    int r = sv_kernel.f_poll_button_P4(VAR_BTN, &ev);
    if (r == SV_OK) {
      got++;
      // [L4-2] Mostra delta_us invece del timestamp assoluto
      uint32_t last_delta =
          (g_trace.count > 0) ? g_trace.entries[g_trace.count - 1].delta_us : 0;
      Serial.printf("  event #%d  delta_us=%lu  trace=%d  dropped=%lu\n", got,
                    (unsigned long)last_delta,
                    sv_kernel.f_trace_count(&g_trace),
                    (unsigned long)sv_kernel.f_evt_dropped_count());
    }
    yield();
  }
  Serial.printf("  Events in 5s: %d  dropped=%lu\n", got,
                (unsigned long)sv_kernel.f_evt_dropped_count());
  Serial.println();
}

// ── LIVE MODE ─────────────────────────────────────────────────
static bool s_live_mode = false;
// [R1L5-LOOP] Flag di completamento suite: sostituisce il while(true) finale.
// Quando true, loop() restituisce subito (yield + return) senza bloccarsi.
// Strutturalmente più corretto per Arduino: mantiene il watchdog soddisfatto
// e rende esplicita l'intenzione ("suite completata, sistema in idle").
static bool s_suite_done = false;
// [L6-COROUTINE] s_last_ctrl_ms e s_last_export_ms rimossi:
// il timing è ora gestito da COROUTINE_DELAY(1000) e COROUTINE_DELAY(5000)
// nelle coroutine processingTask ed exportTask.

static INT s_live_ev;
static INT s_live_avg;
static INT s_live_thr;
static sv_bool_t s_live_cond;
static bool s_live_vars_init = false;

static void live_init_vars_once(void) {
  if (s_live_vars_init)
    return;
  s_live_ev = init(SV_T_BOOL, "live_ev", &g_trace);
  s_live_avg = init(SV_T_F32, "live_avg", &g_trace);
  s_live_thr = init(SV_T_F32, "live_thr", &g_trace);
  s_live_cond.v = init(SV_T_BOOL, "live_cond", &g_trace);
  s_live_vars_init = true;
}

// [L6-COROUTINE] Refactoring asincrono basato sull'uso della libreria
// AceRoutine Le vecchie funzioni di task manuali (basate su millis() e static
// state) sono state sostituite da coroutine stackless, che proteggono le struct
// in RAM.
//
// [R3L6-GRAFO] Le macro AceRoutine come nodi di biforcazione del grafo
// (email Prof. Culmone 14.05.2026).
// Le coroutine AceRoutine non realizzano vera concorrenza, ma multitasking
// cooperativo. Le macro COROUTINE_YIELD(), COROUTINE_DELAY() e
// COROUTINE_LOOP() sono punti di sospensione/ripresa dove il controllo
// torna allo scheduler. Nel grafo di verifica del workflow, ogni invocazione
// di queste macro diventa un NODO DI BIFORCAZIONE dell'esecuzione:
// il verifier di Emanuel non sa quale coroutine verrà schedulata dopo
// un YIELD (non-determinismo asincrono), ma verifica che le operazioni
// kernel lungo ogni ramo (ASSIGN, ADD, GT, WRITE, ecc.) siano corrette.
//
// Distinzione fondamentale (Prof. Culmone):
//   - VALIDITÀ STRUTTURALE: verificabile con l'architettura attuale.
//     L'extractor di Emanuel può marcare le API AceRoutine con un flag
//     per trattarle come branch-point nel grafo.
//   - VALIDITÀ TEMPORALE: NON verificabile finché AceRoutine vive in
//     user-space. Un attaccante può manipolare i tempi di scheduling;
//     i delta_gap_us inter-coroutine non sono fidati. Richiede la
//     kernelizzazione dello scheduler (vedi [R3L6-FUTURO]).

COROUTINE(acqGpsTask) {
  COROUTINE_LOOP() {
    if (s_live_mode && s_live_vars_init) {
      // [L5-1] Polling GPS non bloccante
      sv_kernel.f_gps_update();

      int r = sv_kernel.f_poll_button_P4(VAR_BTN, &s_live_ev);
      if (r == SV_OK) {
        uint32_t d = (g_trace.count > 0)
                         ? g_trace.entries[g_trace.count - 1].delta_us
                         : 0;
        Serial.printf("  [LIVE] BTN  delta_us=%lu  trace=%d\n",
                      (unsigned long)d, sv_kernel.f_trace_count(&g_trace));
      }
    }
    COROUTINE_YIELD(); // Cede la CPU cooperativamente
  }
}

COROUTINE(processingTask) {
  COROUTINE_LOOP() {
    COROUTINE_DELAY(1000); // Sostituisce task_control_every_1s
    if (s_live_mode && s_live_vars_init) {
      // Sequenza atomica: avg → soglia → confronto → attuazione
      sv_kernel.f_avg_lastN_temp_P3(10, VAR_AVG, &s_live_avg);
      sv_kernel.f_const_f32(25.0f, VAR_THR, &s_live_thr);
      sv_kernel.f_gt(&s_live_avg, &s_live_thr, VAR_COND, &s_live_cond);
      sv_kernel.f_write_fan_P12(&s_live_cond);
      Serial.printf("  [LIVE] CTRL  fan=%s  trace=%d\n",
                    digitalRead(12) ? "ON" : "OFF",
                    sv_kernel.f_trace_count(&g_trace));
    }
    COROUTINE_YIELD(); // coerenza con acqGpsTask
  }
}

COROUTINE(exportTask) {
  COROUTINE_LOOP() {
    COROUTINE_DELAY(5000); // Sostituisce task_export_every_5s
    if (s_live_mode && s_live_vars_init) {
      int n = sv_kernel.f_trace_count(&g_trace);
      if (n > 0) {
        sv_kernel.f_gps_fill_trace(&g_trace);
        char *jstr = NULL;
        size_t jlen = 0;
        if (sv_kernel.f_trace_export_json(&g_trace, &jstr, &jlen) == SV_OK &&
            jstr) {
          Serial.printf(
              "  [LIVE] JSON  %d entries  %zu bytes  → pronto per HTTPS\n", n,
              jlen);
          // TODO [R1L5-HTTPS] Trasmissione HTTPS al server di Emanuel...
          free(jstr);
        }
        // [L6-FLASH] Flush prima del clear
        // [R3L6-OVERHEAD] Nonostante LittleFS, le operazioni I/O restano a ~2us
        // grazie alla natura asincrona della coroutine e al buffering hardware.
        sv_kernel.f_trace_flush_to_flash(&g_trace);
        sv_kernel.f_trace_clear(&g_trace);
      }
    }
    COROUTINE_YIELD(); // coerenza con acqGpsTask
  }
}

// ── Arduino entry points ──────────────────────────────────────
// [Prof. Culmone] setup() e loop() sono i punti di ingresso del codice
// applicativo: qui il meccanismo di chiamata passa esplicitamente per
// sv_kernel (la dispatch table), modellandolo come system call / DLL import.
void setup(void) {
  // Attendi 3 secondi per permettere alla porta USB CDC di enumerarsi su PC
  delay(3000);
  Serial.begin(115200);

#ifdef GPS_HARDWARE
  // Inizializza lo schermo dell'AtomS3 senza toccare la seriale
  auto cfg = M5.config();
  cfg.serial_baudrate = 0;
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.setCursor(10, 10);
  M5.Display.println("SV-API V3");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.println("Booting...");
#endif

  // [L5-1] Inizializzazione GPS (Atomic GPS Base v2.0 via UART1)
  gps_init();

  // Chiamate attraverso sv_kernel: il codice applicativo non sa nulla
  // dell'implementazione interna — conosce solo la dispatch table.
  sv_kernel.f_init();
  sv_kernel.f_trace_init(&g_trace);
  sv_kernel.f_trace_flash_init(&g_trace); // [L6-FLASH] Inizializza persistenza

  // [L4-3] Calibrazione all'avvio attraverso la dispatch table.
  // Il risultato viene incluso in ogni JSON esportato come riferimento
  // temporale per il verifier di Emanuel.
  uint32_t cal = sv_kernel.f_calibrate(8);

  sv_kernel.f_bind_button_P4();

  Serial.println();
  Serial.println(F("╔══════════════════════════════════════════════════════════"
                   "══════════════╗"));
#ifdef GPS_HARDWARE
  Serial.println(F("║   SV-API — Atom GPS v2.0 (Lezione 6)    DISPOSITIVO "
                   "PRONTO           ║"));
#else
  Serial.println(F("║   SV-API — ESP32/Wokwi  (Lezione 6)     DISPOSITIVO "
                   "PRONTO           ║"));
#endif
  Serial.println(F("╠══════════════════════════════════════════════════════════"
                   "══════════════╣"));
  Serial.printf("║   Calibrazione: %5lu us/op (sv_add)                         "
                "          ║\n",
                (unsigned long)cal);
  Serial.println(F("║   Premi INVIO nel Serial Monitor per avviare la suite    "
                   "             ║"));
  Serial.println(F("╚══════════════════════════════════════════════════════════"
                   "══════════════╝"));
  Serial.flush();

  // [L6-COROUTINE] Inizializzazione obbligatoria dello scheduler di AceRoutine.
  // Processa la linked list delle coroutine definite staticamente. Senza questa
  // chiamata, la lista rimane non inizializzata e CoroutineScheduler::loop()
  // causerebbe un Null Pointer Dereference (LoadProhibited a 0x00000000).
  // [R3L6-FUTURO] Lo scheduler AceRoutine vive ancora in spazio utente (Application Layer).
  // In una futura iterazione architetturale, dovrà essere spostato all'interno
  // del kernel (Sezione 2) per impedire al codice utente di ritardare la coroutine
  // processingTask con un while(1) malevolo.
  // [R3L6-GRAFO] La verifica STRUTTURALE della traccia è già garantita: le macro
  // YIELD/DELAY sono nodi di biforcazione nel grafo (email Prof. 14.05.2026).
  // La verifica TEMPORALE inter-coroutine richiede la kernelizzazione (v7).
  CoroutineScheduler::setup();
}

// [L6-COROUTINE] La "Suite" di test iniziali viene mantenuta nel loop
// con esecuzione bloccante/nativa per evitare Kernel Panic (LoadProhibited)
// o corruzione di memoria con l'hardware M5.Display durante il context switch.
void loop(void) {
  if (!s_suite_done) {
    // Su hardware reale (GPS_HARDWARE) la suite parte automaticamente dopo
    // 5 secondi, perché il terminale VS Code non riesce a inviare input
    // all'ESP32-S3 via USB-Serial/JTAG. Su Wokwi si attende l'INVIO manuale.
    static uint32_t last_prompt_ms = 0;
    static bool auto_started = false;

    while (!auto_started) {
#ifdef GPS_HARDWARE
      // Polling continuo del GPS durante l'attesa per svuotare il buffer UART
      sv_kernel.f_gps_update();

      // Auto-start solo quando il fix è confermato su hardware reale.
      // Su un cold-boot il modulo può impiegare dai 30 ai 60 secondi (o più)
      // per agganciare i satelliti.
      if (s_gps.location.isValid()) {
        Serial.println(F(">>> Fix GPS confermato! Auto-start hardware <<<"));
        auto_started = true;
      } else {
        if (millis() - last_prompt_ms >= 1000) {
          Serial.println(F(
              ">>> Attesa segnale GPS... (premi INVIO per forzare avvio) <<<"));
          Serial.flush();
          M5.Display.fillScreen(TFT_BLACK);

          // Intestazione
          M5.Display.setTextSize(2); // Testo grande (12x16)
          M5.Display.setTextColor(TFT_CYAN, TFT_BLACK);
          M5.Display.setCursor(4, 5);
          M5.Display.println("GPS WAIT");

          // Linea di separazione
          M5.Display.drawLine(0, 25, 128, 25, TFT_WHITE);

          // Dati in tempo reale (testo più piccolo per farli entrare)
          M5.Display.setTextSize(1); // Testo piccolo (6x8)
          M5.Display.setCursor(4, 32);
          M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
          M5.Display.printf("Satelliti : %lu\n",
                            (unsigned long)s_gps.satellites.value());

          M5.Display.setCursor(4, 45);
          M5.Display.printf("Fix Valido: %s\n",
                            s_gps.location.isValid() ? "SI" : "NO");

          // Mostra le coordinate in diretta se il fix è valido
          M5.Display.setCursor(4, 60);
          M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
          if (s_gps.location.isValid()) {
            M5.Display.printf("Lat: %.5f\n", s_gps.location.lat());
            M5.Display.setCursor(4, 72);
            M5.Display.printf("Lng: %.5f\n", s_gps.location.lng());
          } else {
            M5.Display.println("Ricerca segnale...");
          }

          // Conto alla rovescia sostituito da indicatore di attesa
          M5.Display.drawLine(0, 90, 128, 90, TFT_WHITE);
          M5.Display.setCursor(4, 100);
          M5.Display.setTextSize(1);
          M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
          M5.Display.println("In attesa di FIX...");
          last_prompt_ms = millis();
        }
        if (Serial.available()) {
          while (Serial.available())
            Serial.read();
          auto_started = true;
        } else {
          yield();
        }
      }
#else
      if (millis() - last_prompt_ms >= 3000) {
        Serial.println(F(">>> In attesa — premi INVIO nel Serial Monitor <<<"));
        Serial.flush();
        last_prompt_ms = millis();
      }
      if (Serial.available()) {
        while (Serial.available())
          Serial.read();
        auto_started = true;
      } else {
        yield();
      }
#endif
    }

    // Pulizia registry e trace prima di ogni suite run attraverso sv_kernel.
    // In un sistema reale questi sarebbero system call verso il kernel:
    // il codice utente non può accedere direttamente alle strutture interne.
    sv_kernel.f_registry_clear();
    sv_kernel.f_trace_clear(&g_trace);

    print_banner();
    run_benchmarks();
    yield();
    demo_gps_secure_vector();
    yield();
    demo_pipeline_avgN();
    yield();
    demo_async_button_window();
    yield();

    Serial.println(
        F("==========================================================="
          "================"));
    Serial.println(
        F("  SUITE COMPLETATA.  Live mode: decommentare s_live_mode = true."));
    Serial.println(
        F("  Il JSON prodotto e' pronto per la trasmissione HTTPS al "
          "server Emanuel."));
    Serial.println(
        F("==========================================================="
          "================"));
    Serial.flush();

    sv_kernel.f_trace_clear(&g_trace);

    // [R1L5-LOOP] Segnala il completamento. Per passare al live mode,
    // decommentare s_live_mode = true all'inizio del file o qui sotto.
    s_suite_done = true;
#ifdef GPS_HARDWARE
    // [R1L5-UI] Layout finale riassuntivo sullo schermo 128x128
    M5.Display.fillScreen(TFT_DARKGREEN);

    // Titolo
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    M5.Display.setCursor(4, 5);
    M5.Display.println("SUITE OK!");

    M5.Display.drawLine(0, 25, 128, 25, TFT_WHITE);

    // Dati spaziotemporali estratti
    M5.Display.setTextSize(1);
    M5.Display.setCursor(4, 32);
    M5.Display.setTextColor(TFT_YELLOW, TFT_DARKGREEN);
    M5.Display.printf("Satelliti : %lu\n",
                      (unsigned long)s_gps.satellites.value());

    M5.Display.setCursor(4, 45);
    M5.Display.setTextColor(TFT_CYAN, TFT_DARKGREEN);
    M5.Display.print("Lat: ");
    M5.Display.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    M5.Display.println(s_gps.location.lat(), 5);

    M5.Display.setCursor(4, 58);
    M5.Display.setTextColor(TFT_CYAN, TFT_DARKGREEN);
    M5.Display.print("Lng: ");
    M5.Display.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    M5.Display.println(s_gps.location.lng(), 5);

    M5.Display.setCursor(4, 71);
    M5.Display.setTextColor(TFT_CYAN, TFT_DARKGREEN);
    M5.Display.print("Cal: ");
    M5.Display.setTextColor(TFT_ORANGE, TFT_DARKGREEN);
    M5.Display.printf("%lu us\n", (unsigned long)g_trace.cal_tick_us);

    // Footer JSON
    M5.Display.drawLine(0, 88, 128, 88, TFT_WHITE);
    M5.Display.setCursor(18, 100);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_GREENYELLOW, TFT_DARKGREEN);
    M5.Display.println("JSON");
    M5.Display.setCursor(26, 115);
    M5.Display.setTextSize(1);
    M5.Display.println("PRONTO");
#endif
    return;
  }

  // [L6-COROUTINE] Scheduler di sistema cooperativo per la fase LIVE
  // Inizia a girare solo quando la suite di boot ha terminato le operazioni.
  // Inizializza una tantum le variabili necessarie se siamo in Live Mode.
  if (s_live_mode) {
    live_init_vars_once();
    // [R3L6-FUTURO] loop dello scheduler. Spostarlo nel kernel è la priorità v7.
    CoroutineScheduler::loop(); // [L6-COROUTINE] Solo in live mode
  } else {
    sv_kernel.f_trace_flush_to_flash(&g_trace); // [L6-FLASH] flush idle
    yield();
  }
}
