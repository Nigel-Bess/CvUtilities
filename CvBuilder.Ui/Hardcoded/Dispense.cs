using System.Collections.Immutable;

namespace CvBuilder.Ui.Hardcoded;

public record Dispense(string Name, string HostName, Facility facility)
{
    public ImmutableList<Dispense> Dispenses = ImmutableList.Create<Dispense>([
            new("P2","fulfil@p2-dab",Facility.Pioneer)
        ]);
}
