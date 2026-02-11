using System.Collections.Immutable;

namespace CvBuilder.Ui.Hardcoded;

public record Dispense(string Name, SshLogin Login, Facility Facility)
{
    private static ImmutableList<Dispense> DeployableDispenses = ImmutableList.Create<Dispense>([
            new("P1",new("fulfil@p1-dab.pioneer.fulfil.ai","FreshEngr") ,Facility.Pioneer),
            new("P2",new("fulfil@p2-dab.pioneer.fulfil.ai","FreshEngr") ,Facility.Pioneer),
        ]);
    public static Dispense W2Dab => new("W2", new("fulfil@w2-dab.whisman.fulfil.ai", "FreshEngr"), Facility.Whisman);
    public static Dispense WhismanBuildBox => new("Whisman Build Box", new("fulfil@clip-tb.whisman.fulfil.ai", "FreshEngr"), Facility.Whisman);
    public static IEnumerable<Dispense> GetMachines(Facility facility) => DeployableDispenses.Where(d => d.Facility == facility);
    public override string ToString()
    {
        return $"{Name} DAB";
    }
}
