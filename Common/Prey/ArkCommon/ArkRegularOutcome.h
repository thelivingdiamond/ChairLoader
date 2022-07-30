// Header file automatically created from a PDB.

#pragma once
#include <bitset>

// Header: FromCpp
// Prey/ArkCommon/ArkRegularOutcome.h
class ArkRegularOutcome // Id=8013479 Size=4
{
public:
	static constexpr const uint64_t k_chancesIn100 = 100;
	static constexpr const uint64_t k_streakiness = 4;
	static constexpr const uint64_t k_samplesPerChance = 16;
	static constexpr const uint64_t k_chanceCount = 51;
	static constexpr const uint64_t k_resultsChanceCount = 50;
	static constexpr const uint64_t k_resultsPerSample = 400;
	static constexpr const uint64_t k_totalResultsCount = 320000;
	
	using ResultBits = std::bitset<k_totalResultsCount>;
	
	static constexpr const uint64_t k_invertedBitCount = 1;
	static constexpr const uint64_t k_chanceIndexBitCount = 6;
	static constexpr const uint64_t k_sampleIndexBitCount = 5;
	static constexpr const uint64_t k_resultIndexBitCount = 9;
	static constexpr const uint64_t k_totalBitCount = 21;
	
	using DataType = unsigned;
	
	static constexpr const uint64_t k_invertedShift = 20;
	static constexpr const uint64_t k_chanceIndexShift = 14;
	static constexpr const uint64_t k_sampleIndexShift = 9;
	static constexpr const uint64_t k_resultIndexShift = 0;
	static constexpr const unsigned k_invertedMask = 1048576;
	static constexpr const unsigned k_chanceIndexMask = 1032192;
	static constexpr const unsigned k_sampleIndexMask = 15872;
	static constexpr const unsigned k_resultIndexMask = 511;
	static std::bitset<k_totalResultsCount> s_results;
	unsigned m_data;
	static constexpr const uint64_t k_maxRunCount = 8;
	
	static void Initialize(unsigned _seed) { FInitialize(_seed); }
	ArkRegularOutcome();
	bool operator()() const { return FoperatorRBLRBR(this); }
	void Reset() const { FResetOv1(this); }
	void Reset(float _probability) const { FResetOv0(this,_probability); }
	
#if 0
	void SetInverted(bool arg0) const;
	void SetChanceIndex(uint64_t arg0) const;
	void SetSampleIndex(uint64_t arg0) const;
	void SetResultIndex(uint64_t arg0) const;
	bool GetInverted() const;
	uint64_t GetChanceIndex() const;
	uint64_t GetSampleIndex() const;
	uint64_t GetResultIndex() const;
#endif
	
	static inline auto FInitialize = PreyFunction<void(unsigned _seed)>(0x1056A20);
	static inline auto FoperatorRBLRBR = PreyFunction<bool(ArkRegularOutcome const *const _this)>(0x1056930);
	static inline auto FResetOv1 = PreyFunction<void(ArkRegularOutcome const *const _this)>(0x1056D30);
	static inline auto FResetOv0 = PreyFunction<void(ArkRegularOutcome const *const _this, float _probability)>(0x1056C80);
};

