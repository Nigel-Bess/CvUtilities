namespace CvBuilder.Ui.Hardcoded;

public static class BuildMachine
{
    public static IEnumerable<Dispense> All() => [
            Dispense.BuildBox,
            Dispense.P1,
            Dispense.P2,
        ];
}
