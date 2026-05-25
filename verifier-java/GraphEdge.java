public class GraphEdge {

    public int from;

    public int to;

    public ComplexityMetadata complexity;

    public GraphEdge(
            int from,
            int to,
            ComplexityMetadata complexity
    ) {

        this.from = from;

        this.to = to;

        this.complexity = complexity;
    }

    @Override
    public String toString() {

        return "[Edge "
                + from
                + " -> "
                + to
                + " "
                + complexity
                + "]";
    }
}