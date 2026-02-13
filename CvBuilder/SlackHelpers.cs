using SlackNet;

namespace CvBuilder;

public static class SlackHelpers
{
    public static async Task<List<string>> GetLockEmojiHoldersAsync(string channelId, string messageTimeStamp, string slackToken)
    {

        var api = new SlackApiClient(slackToken);

        // inputs you already have from context / event
        var emojiName = "lock";          // no colons, just the name

        var message = await api.Reactions.GetForMessage(channelId, messageTimeStamp);
        if (!message.Reactions.TryGetFirst(r => r.Name == emojiName, out var lockReact)) return [];


        // If you need user details (display names, etc.), look them up:
        var users = await Task.WhenAll(lockReact.Users.Select(id => api.Users.Info(id)));
        return users.Select(u => u.Profile.DisplayName).ToList();
    }

}
