public class ComplexityMetadata {

    public int branches;

    public int loops;

    public int coroutines;

    public ComplexityMetadata(
            int branches,
            int loops,
            int coroutines
    ) {

        this.branches = branches;

        this.loops = loops;

        this.coroutines = coroutines;
    }

    @Override
    public String toString() {

        return "[branches="
                + branches
                + ", loops="
                + loops
                + ", coroutines="
                + coroutines
                + "]";
    }
}