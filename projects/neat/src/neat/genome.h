#pragma once

#include "neat/neat.h"

class Genome
{
	friend class Neat;
public:
	struct Connection
	{
		bool enabled;
		float weight;
		uint32_t innovation;
	};

	struct CompatibilityStats
	{
		uint32_t m;
		uint32_t n;
		float w;
	};

public:
	Genome(Neat& neat, bool initialize = true);

	void addNode(const Gene& gene);
	void addNode(); // Adds random node
	void addConnection(const Gene& gene, bool enabled = true, float weight = 1.0f);
	void addConnection(); // Adds random connection (skips if everything is connected)
	void forward(const std::vector<float>& input);
	CompatibilityStats calculateCompatibilityStats(const Genome& mate) const;
	Genome crossover(const Genome& mate) const;
	const std::vector<float>& getOutput() const { return output; }
	void setFitness(float fitness) { this->fitness = fitness; }
	uint32_t getMaxInnovation() const;
	bool isCompatible(const CompatibilityStats& stats) const;
	bool isCompatible(const Genome& genome) const;

private:
	void addNode(uint32_t node);
	float generateRandomWeight() const { return RNG::Float() * 2.0 - 1.0; }

private:
	std::vector<uint32_t> nodes; // Not necessary, just speeds up some calculations
	std::unordered_set<uint32_t> nodes_set; // Not necessary, just speeds up some calculations
	std::map<float, std::vector<Gene>> depth_to_genes;
	std::unordered_map<Gene, Connection> connections;
	std::vector<float> output;
	float fitness = 0.0f;
	Neat& neat;
};