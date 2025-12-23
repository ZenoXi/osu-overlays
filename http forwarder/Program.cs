using Newtonsoft.Json;

// Prevent RequestData from being trimmed on publish, leading to deserialization errors
RequestData.PreventTrimming();

string? input = Console.ReadLine();
if (input == null)
    return -10;

RequestData? requestData = JsonConvert.DeserializeObject<RequestData>(input);
if (requestData == null)
    return -11;

if (requestData.BaseUrl == null || requestData.Path == null)
    return -12;

HttpClient client = new HttpClient();
client.BaseAddress = new Uri(requestData.BaseUrl);

var request = new HttpRequestMessage(HttpMethod.Get, requestData.Path);
if (requestData.Headers != null)
{
    foreach (var header in requestData.Headers)
        request.Headers.Add(header.Key, header.Value);
}

var response = await client.SendAsync(request);
if (response == null)
    return -13;

Console.Write((int)response.StatusCode);

if (response.IsSuccessStatusCode)
{
    if (requestData.Headers != null && requestData.Headers.ContainsValue("application/octet-stream"))
    {
        var content = await response.Content.ReadAsByteArrayAsync();
        if (content != null)
            Console.Write(" " + Convert.ToBase64String(content));
    }
    else
    {
        var content = await response.Content.ReadAsStringAsync();
        if (content != null)
            Console.Write(" " + content);
    }

}

Console.WriteLine();
return 0;