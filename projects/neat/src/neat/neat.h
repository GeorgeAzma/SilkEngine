#pragma once
/*
TODO:
	Crossover:
	1. Choose two parents (A and B) to crossover
	2. Iterate with max(A.max_innovation_num, B.max_innovation_num) iterations
	3. Align genes with same innovation numbers
	4. Inherit matching genes from both parents randomly
	5. Inherit excess and disjoint genes from more fit parent (if fitness is unknown these are random too)
	6. Have a set chance to disable gene if either of the parents have it disabled
	   Now you have a new offspring, which might have some disabled genes
	7. Reenable 25% of disabled genes (Not sure about this)

	Speciation:
	1. Measure distance D between two networks, D = c1 * E / N + c2 * D / N + c3 * w
	   E = excess genes, D = disjoint genes, N = max gene count between two networks (for normalizing)
	   w = average weight differences of matching genes
	   c1, c2, c3 = tweakable constants

	Simulation:
	1. Create population
	2. Create fitness function
	3. Make generation system and stuff

	Others:
	1. Add saving
	2. Add rendering
	3. Add ability to remove networks, species, nodes, connections...

UNTESTED:
	Adding a node:
	1. Choose two random nodes (A and B) which do not have same depth value
	2. Create a node (C) in between A and B with depth (A + B) * 0.5
	3. Disable A -> B connection (if exists) and Add A -> C connection with weight 1.0 and C -> B connection with old weight
	4. Register inovation

	Adding a connection:
	1. Choose two random nodes (A and B) which do not have same depth value
	2. If connection does not exist create a new one with random weight
	3. Register inovation

DONE:
	Initializing:
	1. Set input and output node counts (Required)
	2. Change evolution parameters from default (Optional)
*/

class Genome;

struct Gene
{
	uint32_t in = -1;
	uint32_t out = -1;
	bool isInvalid() const { return in == uint32_t(-1); }
	bool isRecurrent() const { return in == out; }
	bool operator==(const Gene& other) const { return other.in == in && other.out == out; }
};

namespace std
{
	template<>
	struct hash<Gene>
	{
		size_t operator()(const Gene& g) const;
	};
}

class Neat
{
	friend class Genome;
public:
	float c1 = 1.0f; // mismatching gene multiplier when calculating compatability distance
	float c2 = 0.4f; // Average weight differences of matching genes multiplier when calculating compatability distance
	float gene_disable_chance = 0.75f; // Chance to disable gene when either of parents have it disabled
	float gene_enable_chance = 0.25f; // Chance to enable gene after crossover
	float compatibility_threshold = 3.0f; // Specifies when to consider network incompatible and split it apart from rest of the networks as a new specie
	float weight_mutation_chance = 0.8f;
	float new_weight_chance = 0.1f;
	float new_node_chance = 0.03f;
	float new_connection_chance = 0.05f;

public:
	Neat(uint32_t input_count, uint32_t output_count);

	void initPopulation(uint32_t population_size = 150);
	void addGenome(const shared<Genome>& genome);
	uint32_t getInputCount() const { return input_count; }
	uint32_t getOutputCount() const { return output_count; }
	uint32_t getOutputIndex() const { return getInputCount(); }
	uint32_t getBiasIndex() const { return getInputCount() + getOutputCount(); }

private:
	uint32_t addNode(float depth);
	uint32_t addNode(const Gene& gene);
	uint32_t addConnection(const Gene& gene);
	float getDepthInBetweenNodes(const Gene& gene) const;
	const Gene& getGene(size_t innovation) const { return genes[innovation]; }
	float getNodeDepth(size_t node_index) const { return node_depths[node_index]; }
	constexpr float getInputLayerDepth() const { return 0.0f; }
	constexpr float getOutputLayerDepth() const { return 1.0f; }
	bool isSameDepth(uint32_t node1, uint32_t node2) const { return getNodeDepth(node1) == getNodeDepth(node2); }
	bool isOutputNode(uint32_t node) const { return getNodeDepth(node) == getOutputLayerDepth(); }

private:
	std::vector<float> node_depths;
	std::vector<std::vector<shared<Genome>>> species;
	std::vector<Gene> genes;
	std::unordered_set<Gene> genes_set;
	uint32_t input_count;
	uint32_t output_count;
};