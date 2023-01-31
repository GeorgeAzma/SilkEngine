#pragma once
#undef ERROR

class Fence : NonCopyable
{
public:
	enum State
	{
		SIGNALED = VK_SUCCESS,
		UNSIGNALED = VK_NOT_READY,
		ERROR = VK_ERROR_DEVICE_LOST,
	};

public:
	Fence(bool signaled = false);
	~Fence();

	State getState() const;
	void reset() const;
	void wait() const;

	operator const VkFence& () const { return fence; }

private:
	VkFence fence = nullptr;
	mutable bool signaled = false;
};