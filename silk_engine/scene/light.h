#pragma once

struct Light
{
    vec3 position = vec3(0);
    float linear = 0.0;
    vec3 direction = vec3(0);
    float quadratic = 0.0;
    alignas(16) vec3 color = vec3(0);

    bool operator==(const Light& other) const
    {
        return position == other.position
            && linear == other.linear
            && direction == other.direction
            && quadratic == other.quadratic
            && color == other.color;
    }
};