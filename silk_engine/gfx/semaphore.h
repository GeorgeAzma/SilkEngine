#pragma once

class Semaphore
{
public:
	Semaphore();
	~Semaphore();

	void signal(uint64_t value) const;

	operator const VkSemaphore& () const { return semaphore; }

private:
	VkSemaphore semaphore = nullptr;
};