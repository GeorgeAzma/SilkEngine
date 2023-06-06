#pragma once

class Semaphore
{
public:
	Semaphore();
	~Semaphore();

	void signal(uint64_t value) const;
	void wait(uint64_t value = 0, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const;

	operator const VkSemaphore& () const { return semaphore; }

private:
	VkSemaphore semaphore = nullptr;
};