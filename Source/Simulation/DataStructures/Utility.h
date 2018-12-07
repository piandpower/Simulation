#pragma once

#include <algorithm>
#include <vector>
#include <future>
#include <numeric>

#include "SceneComponents/Particle.h"
#include "Runtime/Core/Public/Async/Async.h"

#include "CoreMinimal.h"


template <typename Item, typename Value>
inline Value ParallelSum(const std::vector<Item>& vector, std::function<Value(Item)> valueGetter) {

	// Determine total size.
	const unsigned int size = vector.size();

	// Determine how many parts the work shall be split into.
	const unsigned int parts = std::thread::hardware_concurrency();

	std::vector<TFuture<Value>> futures;
	futures.reserve(parts);
	auto first = vector.begin();

	// For each part, calculate size and run accumulate on a separate thread.
	for (std::size_t i = 0; i != parts; ++i) {
		const auto partSize = (size * i + size) / parts - (size * i) / parts;

		futures.emplace_back(Async<Value>(EAsyncExecution::TaskGraph, [first, partSize, &valueGetter]() -> Value {
			return std::accumulate(first, std::next(first, partSize), 0.0, [&valueGetter](const Value prev, const Item next) -> Value { return prev + valueGetter(next); });
		}));
		std::advance(first, partSize);
	}

	// Wait for all threads to finish execution and accumulate results.
	return std::accumulate(std::begin(futures), std::end(futures), 0.0,
		[](const Value prev, TFuture<Value>& future) { return prev + future.Get(); });
}


template <typename Item>
inline Item ParallelSum(const std::vector<Item>& vector) {
	return ParallelSum<Item, Item>(vector, [&](Item item) { return item; });
}

template <typename Item, typename Value>
inline Value ParallelMax(const std::vector<Item>& vector, std::function<Value(Item)> valueGetter, int minElementsForParallel = 10000) {

	if (vector.size() == 0)
		throw("Vector shall not be empty");

	if (vector.size() < minElementsForParallel || std::thread::hardware_concurrency() >= vector.size()) {
		return valueGetter(*std::max_element(std::begin(vector), std::end(vector), [&valueGetter](Item p1, Item p2) -> bool { return valueGetter(p1) < valueGetter(p2); }));
	}

	// Determine how many parts the work shall be split into.
	const unsigned int parts = std::thread::hardware_concurrency();

	std::vector<std::shared_future<Value>> futures;
	futures.reserve(parts);
	std::vector<Item>::const_iterator first = vector.begin();

	// For each part, calculate size and run max on a separate thread.
	for (int i = 0; i != parts; ++i) {
		const unsigned int partSize = (vector.size() * i + vector.size()) / parts - (vector.size() * i) / parts;
		auto end = std::next(first, partSize);
		futures.emplace_back(std::async(std::launch::async, [first, partSize, &vector, &valueGetter]() -> Value {

			std::vector<Item>::const_iterator end = std::next(first, partSize);
			return valueGetter(*std::max_element(first, end, [&valueGetter](Item p1, Item p2) -> bool { return valueGetter(p1) < valueGetter(p2);
			}));
		}));

		std::advance(first, partSize);
	}

	// Wait for all threads to finish and search the max result
	return std::max_element(futures.begin(), futures.end(), [&valueGetter](std::shared_future<Value> f1, std::shared_future<Value> f2) -> bool { return f1.get() < f2.get(); })->get();
}

template <typename Item>
inline Item ParallelMax(const std::vector<Item>& vector) {
	
	if (vector.size() == 0) 
		throw("Cant find maximum element in empty vector");

	return ParallelMax<Item, Item>(vector, [](Item item) { return item; });
}

template <typename Item, typename Value>
inline Value ParallelMin(const std::vector<Item>& vector, std::function<Value(Item)> valueGetter, int minElementsForParallel = 10000) {

	if (vector.size() == 0)
		throw("Vector shall not be empty");

	if (vector.size() < minElementsForParallel || std::thread::hardware_concurrency() >= vector.size()) {
		return valueGetter(*std::min_element(std::begin(vector), std::end(vector), [&valueGetter](Item p1, Item p2) -> bool { return valueGetter(p1) < valueGetter(p2); }));
	}

	// Determine how many parts the work shall be split into.
	const unsigned int parts = std::thread::hardware_concurrency();

	std::vector<std::shared_future<Value>> futures;
	futures.reserve(parts);
	std::vector<Item>::const_iterator first = vector.begin();

	// For each part, calculate size and run max on a separate thread.
	for (int i = 0; i != parts; ++i) {
		const unsigned int partSize = (vector.size() * i + vector.size()) / parts - (vector.size() * i) / parts;
		auto end = std::next(first, partSize);
		futures.emplace_back(std::async(std::launch::async, [first, partSize, &vector, &valueGetter]() -> Value {

			std::vector<Item>::const_iterator end = std::next(first, partSize);
			return valueGetter(*std::min_element(first, end, [&valueGetter](Item p1, Item p2) -> bool { return valueGetter(p1) < valueGetter(p2);
			}));
		}));

		std::advance(first, partSize);
	}

	// Wait for all threads to finish and search the max result
	return std::min_element(futures.begin(), futures.end(), [&valueGetter](std::shared_future<Value> f1, std::shared_future<Value> f2) -> bool { return f1.get() < f2.get(); })->get();
}

template <typename Item>
inline Item ParallelMin(const std::vector<Item>& vector) {

	if (vector.size() == 0)
		throw("Cant find maximum element in empty vector");

	return ParallelMin<Item, Item>(vector, [](Item item) { return item; });
}

template <typename Item>
inline bool ParallelExists(const std::vector<Item>& vector, std::function<bool(Item)> valueGetter, int minElementsForParallel = 10000) {

	if (vector.size() == 0) {
		return false;
	}

	if (vector.size() < minElementsForParallel || std::thread::hardware_concurrency() >= vector.size()) {
		for (const Item& item : vector) {
			if (valueGetter(item)) {
				return true;
			}
		}
	}

	// Determine how many parts the work shall be split into.
	const unsigned int parts = std::thread::hardware_concurrency();

	std::vector<std::future<bool>> futures;
	futures.reserve(parts);
	std::vector<Item>::const_iterator first = vector.begin();

	// For each part, calculate size and run max on a separate thread.
	for (int i = 0; i != parts; ++i) {
		const unsigned int partSize = (vector.size() * i + vector.size()) / parts - (vector.size() * i) / parts;
		auto end = std::next(first, partSize);
		futures.emplace_back(std::async(std::launch::async, [first, partSize, &vector, &valueGetter]() -> bool {

			std::vector<Item>::const_iterator end = std::next(first, partSize);
			for (auto iterator = first; iterator != end; iterator++) {
				if (valueGetter(*iterator)) {
					return true;
}
			}
			return false;
		}));

		std::advance(first, partSize);
	}

	// Wait for all threads to finish and search a positive result
	for (std::future<bool>& fut : futures) {
		if (fut.get()) {
			return true;
		}
	}
	return false;
}