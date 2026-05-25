public class GraphNode {

    public int id;

    public String type;

    public String name;

    public GraphNode(
            int id,
            String type,
            String name
    ) {

        this.id = id;

        this.type = type;

        this.name = name;
    }

    @Override
    public String toString() {

        return "[Node id="
                + id
                + ", type="
                + type
                + ", name="
                + name
                + "]";
    }
}