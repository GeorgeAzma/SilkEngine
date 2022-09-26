#include "genome.h"

Genome::Genome(Neat& neat, bool initialize) : neat(neat), output(neat.getOutputCount())
{
	if (initialize)
	{
		for (uint32_t i = 0; i < neat.getInputCount(); ++i)
			for (uint32_t j = 0; j < neat.getOutputCount(); ++j)
				addConnection(Gene{ i, neat.getOutputIndex() + j }, true, generateRandomWeight());
	}
	addNode(neat.getBiasIndex());
}

void Genome::addNode(const Gene& gene)
{
	if (gene.isRecurrent())
		return;

	// Disable existing connection
	Connection& old_connection = connections.at(gene);
	old_connection.enabled = false;

	// Add new node and create two new connections connecting it
	uint32_t new_node_index = neat.addNode(gene);
	Gene g1 = { gene.in, new_node_index };
	addConnection(g1, true, 1.0f); // TODO: Maybe if old connection was disabled, disable these too?
	Gene g2 = { new_node_index, gene.out };
	addConnection(g2, true, old_connection.weight);
}

void Genome::addNode()
{
	const Connection& random_connection = std::next(connections.begin(), RNG::Uint() % connections.size())->second;
	const Gene& g = neat.getGene(random_connection.innovation);
	return addNode(g);
}

void Genome::addConnection(const Gene& gene, bool enabled, float weight)
{
	uint32_t innovation = neat.addConnection(gene);
	if (connections.emplace(gene, Connection{ enabled, weight, innovation }).second)
	{
		depth_to_genes[neat.getNodeDepth(gene.in)].emplace_back(gene);

		addNode(gene.in);
		addNode(gene.out);
	}
}

void Genome::addConnection() // TODO: Make this fast/reliable
{
	for (size_t i = 0; i < nodes.size() * 8; ++i)
	{
		uint32_t n1 = nodes[RNG::Uint() % nodes.size()];
		uint32_t n2 = nodes[RNG::Uint() % nodes.size()];
		if (n1 == n2 || neat.isSameDepth(n1, n2) || connections.find({ n1, n2 }) != connections.end())
			continue;
		addConnection({ n1, n2 }, true, generateRandomWeight());
		break;
	}
}

void Genome::forward(const std::vector<float>& input)
{
	std::unordered_map<uint32_t, float> nodes;
	for (size_t i = 0; i < neat.getInputCount(); ++i)
		nodes[i] = input[i];
	nodes[neat.getBiasIndex()] = 1.0f;
	for (auto&& [depth, genes] : depth_to_genes)
	{
		for (const auto& gene : genes)
		{
			const Connection& connection = connections.at(gene);
			if (connection.enabled)
			{
				const Gene& g = neat.getGene(connection.innovation);
				nodes[g.out] += nodes[g.in] * connection.weight;
				if (neat.isOutputNode(g.out))
					output[g.out - neat.getOutputIndex()] = nodes[g.out];
			}
		}
	}
}

Genome::CompatibilityStats Genome::calculateCompatibilityStats(const Genome& mate) const
{
	uint32_t max_innovation = max(getMaxInnovation(), mate.getMaxInnovation());
	std::vector<Gene> genome(max_innovation);
	for (auto&& [gene, connection] : connections)
		genome[connection.innovation] = gene;
	std::vector<Gene> mate_genome(max_innovation);
	for (auto&& [gene, connection] : mate.connections)
		mate_genome[connection.innovation] = gene;

	uint32_t matching = 0;
	float avarage_weight_difference = 0.0f;
	for (uint32_t i = 0; i < max_innovation; ++i)
	{
		if (genome[i] == mate_genome[i])
		{
			const Connection& con = connections.at(genome[i]);
			const Connection& mate_con = connections.at(mate_genome[i]);
			avarage_weight_difference += mate_con.weight - con.weight;
			++matching;
		}
	}
	avarage_weight_difference /= matching;

	uint32_t mismatching = max_innovation - matching;
	return CompatibilityStats(mismatching, max_innovation, avarage_weight_difference);
}

Genome Genome::crossover(const Genome& mate) const
{
	size_t max_innovation = max(getMaxInnovation(), mate.getMaxInnovation());
	std::vector<Gene> genome(max_innovation);
	for (auto&& [gene, connection] : connections)
		genome[connection.innovation] = gene;
	std::vector<Gene> mate_genome(max_innovation);
	for (auto&& [gene, connection] : mate.connections)
		mate_genome[connection.innovation] = gene;

	Genome offspring(neat, false);

	for (size_t i = 0; i < max_innovation; ++i)
	{
		if (genome[i].isInvalid() || mate_genome[i].isInvalid()) // Mismatching
		{
			if (fitness > mate.fitness)
			{
				if (genome[i].isInvalid())
					continue;
				const Connection& connection = connections.at(genome[i]);
				offspring.addConnection(genome[i], connection.enabled, connection.weight);
			}
			else if (mate.fitness > fitness)
			{
				if (mate_genome[i].isInvalid())
					continue;
				const Connection& connection = mate.connections.at(mate_genome[i]);
				offspring.addConnection(mate_genome[i], connection.enabled, connection.weight);
			}
			else
			{
				bool rng = RNG::Bool();
				const Gene& g = rng ? genome[i] : mate_genome[i];
				if (g.isInvalid())
					continue;
				const Connection& connection = (rng ? connections : mate.connections).at(g);
				offspring.addConnection(g, connection.enabled, connection.weight);
			}
			continue;
		}
		// Matching
		const Connection& connection = connections.at(genome[i]);
		const Connection& mate_connection = mate.connections.at(mate_genome[i]);
		bool enabled = (connection.enabled && mate_connection.enabled) || RNG::Float() > neat.gene_disable_chance;
		if (RNG::Bool())
			offspring.addConnection(genome[i], enabled, connection.weight);
		else
			offspring.addConnection(mate_genome[i], enabled, mate_connection.weight);
	}

	return offspring;
}

uint32_t Genome::getMaxInnovation() const
{
	uint32_t max_innovation = 0;
	for (auto&& [gene, connection] : connections)
		max_innovation = max(max_innovation, connection.innovation + 1);
	return max_innovation;
}

bool Genome::isCompatible(const CompatibilityStats& stats) const
{
	return (neat.c1 * float(stats.m) / float(stats.n) + neat.c2 * stats.w) <= neat.compatibility_threshold;
}

bool Genome::isCompatible(const Genome& genome) const
{
	return isCompatible(calculateCompatibilityStats(genome));
}

void Genome::addNode(uint32_t node)
{
	if (!nodes_set.contains(node))
	{
		nodes_set.emplace(node);
		nodes.emplace_back(node);
	}
}