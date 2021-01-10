#pragma once
#include <chrono>

// From ChiliTimer.h
// Source: https://github.com/planetchili/hw3d

class Timer
{
public:
	Timer() noexcept;
	float Mark() noexcept;
	float Peek() const noexcept;
private:
	std::chrono::steady_clock::time_point last;
};