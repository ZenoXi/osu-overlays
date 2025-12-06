#define D2D_INPUT_COUNT 1
#define D2D_INPUT0_SIMPLE
#define D2D_REQUIRES_SCENE_POSITION
#include "d2d1effecthelpers.hlsli"

cbuffer constants : register(b0)
{
	float4 headColor;
	float trailWidth;
	float headSize;
	float trailEdgeWidth;
	float headEdgeWidth;
	int pointCount;
	int stopCount;
	float colorCycleDuration;
	int trailResolution;
	float4 points[512];
	float4 pointData[512];
	float4 stops[512];
	float4 stopData[512];
};

float2 perpL(float2 v)
{
	return float2(v.y, -v.x);
}

float2 perpR(float2 v)
{
	return float2(-v.y, v.x);
}

float lenSqr(float2 v)
{
	return v.x * v.x + v.y * v.y;
}

float calcColorStrength(float distance, float targetDistance, float edgeWidth)
{
	if (distance > targetDistance)
		return 0.0f;
	else if (distance <= targetDistance - edgeWidth)
		return 1.0f;
	else
	{
		float str = 1.0f - (distance - (targetDistance - edgeWidth)) / edgeWidth;
		return cos(str * 3.14159f) / -2 + 0.5f;
	}
}

float4 colorAtTime(float time)
{
	if (stopCount == 0)
	{
		// Blink black/red to signify error
		return float4((time % 1.0 < 0.5) ? 1.0f : 0.0f, 0.0f, 0.0f, 1.0f);
	}

	float pos = (time / colorCycleDuration) % 1.0f;
	if (pos <= stopData[0].x)
	{
		return stops[0];
	}
	else if (pos >= stopData[stopCount - 1].x)
	{
		return stops[stopCount - 1];
	}
	else
	{
		int index = 1;
		while (pos > stopData[index].x && index < stopCount - 1)
			index++;

		float4 color1 = stops[index - 1];
		float4 color2 = stops[index];
		float x = (pos - stopData[index - 1].x) / (stopData[index].x - stopData[index - 1].x);
		return lerp(color1, color2, x);
	}
}

D2D_PS_ENTRY(D2D_ENTRY)
{
	float2 pixelPos = D2DGetScenePosition().xy;

	bool near = false;
	float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float maxColorStrength = 0.0f;
	float timeAtMaxStrength = 0.0f;
	float halfWidth = trailWidth / 2;

	for (int i = 1; i < pointCount; i++)
	{
		float2 p0 = points[i - 1].xy;
		float2 p1 = points[i].zw;
		float2 p2 = points[i].xy;

		if (pixelPos.x < min(min(p0.x, p1.x), p2.x) - halfWidth ||
			pixelPos.y < min(min(p0.y, p1.y), p2.y) - halfWidth ||
			pixelPos.x > max(max(p0.x, p1.x), p2.x) + halfWidth ||
			pixelPos.y > max(max(p0.y, p1.y), p2.y) + halfWidth)
		{
			if (near)
			{
				near = false;
				float4 trailColor = colorAtTime(timeAtMaxStrength) * maxColorStrength;
				finalColor.x = trailColor.x + finalColor.x * (1.0f - trailColor.a);
				finalColor.y = trailColor.y + finalColor.y * (1.0f - trailColor.a);
				finalColor.z = trailColor.z + finalColor.z * (1.0f - trailColor.a);
				finalColor.a = trailColor.a + finalColor.a * (1.0f - trailColor.a);
			}
			continue;
		}

		float startWidth = (i - 1) / (float)(pointCount - 1) * trailWidth;
		float endWidth = i / (float)(pointCount - 1) * trailWidth;
		float startTime = pointData[i - 1].x;
		float endTime = pointData[i].x;

		// Subdivide single bezier curve into small discrete sections to simplify math
		for (int j = 1; j <= trailResolution; j++)
		{
			float x0 = (float)(j - 1) / (float)trailResolution;
			float x1 = (float)j / (float)trailResolution;
			float2 sp0 = p1 + (1.0f - x0) * (1.0f - x0) * (p0 - p1) + (x0 * x0) * (p2 - p1);
			float2 sp1 = p1 + (1.0f - x1) * (1.0f - x1) * (p0 - p1) + (x1 * x1) * (p2 - p1);

			float str = 0.0f;
			float time = 0.0f;

			// Between subsegment start and end points
			float2 normalToSp1 = normalize(sp1 - sp0);
			float offsetFromStartToEnd = dot(pixelPos - sp0, normalToSp1);
			float offsetFromEndToStart = dot(pixelPos - sp1, -normalToSp1);
			if (offsetFromStartToEnd >= 0.0f && offsetFromEndToStart >= 0.0f)
			{
				float distToSubsegmentCenter = abs(dot(pixelPos - sp0, perpL(normalToSp1)));
				float x = lerp(x0, x1, offsetFromStartToEnd / distance(sp1, sp0));
				float targetWidth = lerp(startWidth, endWidth, x);

				str = calcColorStrength(distToSubsegmentCenter, targetWidth / 2, trailEdgeWidth);
				time = lerp(startTime, endTime, x);
			}
			// At either end
			else
			{
				float targetWidth0 = lerp(startWidth, endWidth, x0);
				float targetWidth1 = lerp(startWidth, endWidth, x1);
				// Start point
				if (lenSqr(pixelPos - sp0) <= (targetWidth0 * targetWidth0))
				{
					str = calcColorStrength(length(pixelPos - sp0), targetWidth0 / 2, trailEdgeWidth);
					time = lerp(startTime, endTime, x0);
				}
				// End point
				else if (lenSqr(pixelPos - sp1) <= (targetWidth1 * targetWidth1))
				{
					str = calcColorStrength(length(pixelPos - sp1), targetWidth1 / 2, trailEdgeWidth);
					time = lerp(startTime, endTime, x1);
				}
			}


			if (str > 0.0f && !near)
			{
				near = true;
				maxColorStrength = str;
				timeAtMaxStrength = time;
			}
			else if (str == 0.0f && near)
			{
				near = false;
				float4 trailColor = colorAtTime(timeAtMaxStrength) * maxColorStrength;
				finalColor.x = trailColor.x + finalColor.x * (1.0f - trailColor.a);
				finalColor.y = trailColor.y + finalColor.y * (1.0f - trailColor.a);
				finalColor.z = trailColor.z + finalColor.z * (1.0f - trailColor.a);
				finalColor.a = trailColor.a + finalColor.a * (1.0f - trailColor.a);
				maxColorStrength = 0.0f;
			}
			else
			{
				if (str > maxColorStrength)
				{
					maxColorStrength = str;
					timeAtMaxStrength = time;
				}
			}
		}
	}

	if (near)
	{
		float4 trailColor = colorAtTime(timeAtMaxStrength) * maxColorStrength;
		finalColor.x = trailColor.x + finalColor.x * (1.0f - trailColor.a);
		finalColor.y = trailColor.y + finalColor.y * (1.0f - trailColor.a);
		finalColor.z = trailColor.z + finalColor.z * (1.0f - trailColor.a);
		finalColor.a = trailColor.a + finalColor.a * (1.0f - trailColor.a);
	}

	// Add cursor head
	if (pointCount > 1)
	{
		float distanceToHead = distance(pixelPos, points[pointCount - 1].xy);
		float colorStrength = calcColorStrength(distanceToHead, headSize / 2, headEdgeWidth);
		if (colorStrength > 0.0f)
		{
			float4 finalHeadColor = headColor * colorStrength;
			finalColor.x = finalHeadColor.x + finalColor.x * (1.0f - finalHeadColor.a);
			finalColor.y = finalHeadColor.y + finalColor.y * (1.0f - finalHeadColor.a);
			finalColor.z = finalHeadColor.z + finalColor.z * (1.0f - finalHeadColor.a);
			finalColor.a = finalHeadColor.a + finalColor.a * (1.0f - finalHeadColor.a);
		}
	}

	return finalColor;
}