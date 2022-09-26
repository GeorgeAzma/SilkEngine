#include "neat.h"
#include "genome.h"

namespace std
{
	size_t hash<Gene>::operator()(const Gene& g) const
	{
		if constexpr (sizeof(size_t) >= sizeof(g.in) * 2)
			return g.in | (g.out << sizeof(g.out));
		else
			return g.in ^ g.out;
	}
}

Neat::Neat(uint32_t input_count, uint32_t output_count)
	: input_count(input_count), output_count(output_count)
{ 
	// Adds required default input/output nodes
	for (size_t i = 0; i < input_count; ++i)
		addNode(getInputLayerDepth());
	for (size_t i = 0; i < output_count; ++i)
		addNode(getOutputLayerDepth());
	addNode(getInputLayerDepth()); // Bias
}

void Neat::initPopulation(uint32_t population_size)
{
}

void Neat::addGenome(const shared<Genome>& genome)
{
	for (auto& genomes : species)
	{
		if (genomes.empty())
			continue;
		if (genomes[0]->isCompatible(*genome)) // TODO: maybe use random instead of 0
		{
			genomes.emplace_back(genome);
			return;
		}
	}
	species.emplace_back(std::vector<shared<Genome>>{ genome });
}

float Neat::getDepthInBetweenNodes(const Gene& gene) const
{
	return (node_depths[gene.in] + node_depths[gene.out]) * 0.5f;
}

uint32_t Neat::addNode(float depth)
{
	node_depths.emplace_back(depth);
	return node_depths.size() - 1;
}

uint32_t Neat::addNode(const Gene& gene)
{
	return addNode(getDepthInBetweenNodes(gene));
}

uint32_t Neat::addConnection(const Gene& gene)
{
	if (auto it = genes_set.find(gene); it != genes_set.end())
		return std::distance(genes_set.begin(), genes_set.find(gene));

	genes_set.emplace(gene);
	genes.emplace_back(gene);
	return genes.size() - 1;
}