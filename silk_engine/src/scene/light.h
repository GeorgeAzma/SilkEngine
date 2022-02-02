#pragma once

struct Light
{
    glm::vec3 position = glm::vec3(0);
    float linear = 0.0;
    glm::vec3 direction = glm::vec3(0);
    float quadratic = 0.0;
    glm::vec3 color = glm::vec3(0);
    float padding;

    bool operator==(const Light& other) const
    {
        return position == other.position
            && linear == other.linear
            && direction == other.direction
            && quadratic == other.quadratic
            && color == other.color;
    }
};