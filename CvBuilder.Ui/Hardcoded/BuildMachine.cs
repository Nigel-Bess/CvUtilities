namespace CvBuilder.Ui.Hardcoded;

public static class BuildMachine
{
    public static IEnumerable<Dispense> All()
    {
        yield return Dispense.WhismanBuildBox;
        yield return Dispense.W2Dab;
    }
}
