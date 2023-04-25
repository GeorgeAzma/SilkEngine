#pragma once

class Semaphore
{
public:
	Semaphore();
	~Semaphore();

	void signal(uint64_t value) const;
	void wait();

	operator const VkSemaphore& () const { return semaphore; }

private:
	VkSemaphore semaphore = nullptr;
};