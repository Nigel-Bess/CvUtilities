using System.Collections.Immutable;

namespace CvBuilder.Ui.Hardcoded;

public record Dispense(string Name, SshLogin Login, Facility Facility)
{
    private static ImmutableList<Dispense> DeployableDispenses = ImmutableList.Create<Dispense>([
            new("P1",new("fulfil@p1-dab.pioneer.fulfil.ai","FreshEngr") ,Facility.Pioneer),
            new("P2",new("fulfil@p2-dab.pioneer.fulfil.ai","FreshEngr") ,Facility.Pioneer),
            new("C1",new("fulfil@c1-dab.tan.fulfil.ai","FreshEngr") ,Facility.Tan),
            new("C2",new("fulfil@c2-dab.tan.fulfil.ai","FreshEngr") ,Facility.Tan),
            new("C3",new("fulfil@c3-dab.tan.fulfil.ai","FreshEngr") ,Facility.Tan),
            new("A1",new("fulfil@a1-dab.tan.fulfil.ai","FreshEngr") ,Facility.Tan),
            new("A2",new("fulfil@a2-dab.tan.fulfil.ai","FreshEngr") ,Facility.Tan),
            new("A3",new("fulfil@a3-dab.tan.fulfil.ai","FreshEngr") ,Facility.Tan),
        ]);
    public static Dispense Cliptb => new("Whisman Build Box", new("fulfil@clip-tb.pioneer.fulfil.ai", "FreshEngr"), Facility.Whisman);
    public static IEnumerable<Dispense> GetMachines(Facility facility) => DeployableDispenses.Where(d => d.Facility == facility);
    public override string ToString()
    {
        return $"{Name} DAB";
    }
}
