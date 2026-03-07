#include "api.h"

int main() {

    int_var a;
    int_var b;
    int_var result;

    api_init(a,"a");
    api_init(b,"b");
    api_init(result,"result");

    api_assign(a,10);
    api_assign(b,5);

    api_add(a,b,result);

    api_div(result,b,result);

    return 0;
}