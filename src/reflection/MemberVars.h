#pragma once

#include "VariableId.h"
#include <string>
#include <set>

enum ExportFlags
{
	ExportFlags_None,
	ExportFlags_Angle,
	ExportFlags_ColorWheel
};

struct ExportInfo final
{
	std::string Unit{ };
	float Min = 0.0f;
	float Max = 0.0f;
	float Speed = 0.0f;
	uint32_t flags = 0;
	std::string Group{};
};

class MemberId final
{
public:

	struct MemberInfo
	{
		std::string Name		{ };
		VariableId	VariableId	{ };
		uint32_t	Offset		{ };
		uint32_t	Size		{ };
		uint32_t	Align		{ };

		// New Stuff
		ExportInfo ExportInfo	{ };

		constexpr bool operator<(const MemberInfo& rhs) const
		{
			return Offset < rhs.Offset;
		}
	};

public:

	constexpr MemberId(TypeId id, uint32_t Offset) : Id{ id }, Offset{ Offset } { };

	constexpr MemberId()									= default;
	constexpr ~MemberId()								= default;
	constexpr MemberId(const MemberId&)					= default;
	constexpr MemberId& operator=(const MemberId&)		= default;
	constexpr MemberId(MemberId&&) noexcept				= default;
	constexpr MemberId& operator=(MemberId&&) noexcept	= default;

public:

	constexpr TypeId	GetTypeId		()	const	{ return Id; }
	constexpr uint32_t	GetOffset		()	const	{ return Offset; }

	const MemberInfo&	GetMemberInfo	()	const;	
	const std::string&	GetName			()	const	{ return GetMemberInfo().Name; }
	VariableId			GetVariableId	()	const	{ return GetMemberInfo().VariableId; }
	uint32_t			GetSize			()	const	{ return GetMemberInfo().Size; }
	uint32_t			GetAlignment	()	const	{ return GetMemberInfo().Align; }

public:

	template <typename Class, typename Field>
	static MemberId RegisterField(const std::string& fieldName, uint32_t Offset, ExportInfo ExportInfo);

	template <typename Field>
	static MemberId RegisterField(TypeId classId, const std::string& fieldName, uint32_t Offset, ExportInfo ExportInfo);

	static MemberId RegisterField(TypeId classId, VariableId MemberId, const std::string& fieldName, uint32_t Offset, uint32_t Size, uint32_t Align, ExportInfo ExportInfo);

	static const std::set<MemberInfo>& GetMemberInfos(TypeId id) { return GetStatics().MemberInfos[id]; }

	static const MemberInfo& GetMemberInfo(TypeId id, const std::string& FieldName);

	static const auto& GetAllMemberInfos() { return GetStatics().MemberInfos; }

	static bool Exists(TypeId classId) { return GetStatics().MemberInfos.contains(classId); }


private:

	struct StaticData
	{
		std::unordered_map<TypeId, std::set<MemberInfo>> MemberInfos{};
		std::unordered_map<TypeId, std::unordered_map<std::string, uint32_t>> MemberInfoNameMap{};
	};

	static StaticData& GetStatics()
	{
		static StaticData data{};
		return data;
	}

private:

	TypeId		Id		{ };
	uint32_t	Offset	{ };

};

struct RegisterMember final
{
	RegisterMember(TypeId classId, VariableId MemberId, const std::string& fieldName, uint32_t Offset, uint32_t Size, uint32_t Align, ExportInfo ExportInfo)
	{
		MemberId::RegisterField(classId, MemberId, fieldName, Offset, Size, Align, ExportInfo);
	}
};

// Helper for accessing member pointers (works with private members when called from inside the class)
template<typename Class, typename Field>
struct MemberPointerHelper
{
	static constexpr uint32_t GetOffset(Field Class::* member_ptr)
	{
		// Get offset using member pointer arithmetic
		return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&(reinterpret_cast<Class*>(nullptr)->*member_ptr)));
	}
};

// Macro to be used INSIDE a class definition to register private members
// Usage in private section of class:
//   REGISTER_MEMBER_PRIVATE(ClassName, fieldName)
#define REGISTER_MEMBER_PRIVATE(TYPE, FIELD, EXPORT_SETTINGS) \
	inline static const RegisterMember TYPE##FIELD##_reg{TypeId::Create<TYPE>(), VariableId::Create<decltype(FIELD)>(), #FIELD, MemberPointerHelper<TYPE, decltype(FIELD)>::GetOffset(&TYPE::FIELD), sizeof(decltype(FIELD)), alignof(decltype(FIELD)), EXPORT_SETTINGS}; \
	static_assert(true, "")

// Original macro for public members (unchanged)
#define REGISTER_MEMBER(TYPE, FIELD, EXPORT_SETTINGS) inline static RegisterMember TYPE##FIELD{TypeId::Create<TYPE>(), VariableId::Create<decltype(TYPE::FIELD)>(), #FIELD, offsetof(TYPE, FIELD), sizeof(decltype(TYPE::FIELD)), alignof(decltype(TYPE::FIELD)), EXPORT_SETTINGS};

inline const MemberId::MemberInfo& MemberId::GetMemberInfo() const
{
	auto& MemberInfos = GetMemberInfos(Id);

	MemberInfo InfoToFind{};
	InfoToFind.Offset = Offset;

	auto it = MemberInfos.find(InfoToFind);

	// should always contain the value, else something went wrong
	assert(it != MemberInfos.end());

	return *it;
}

inline MemberId MemberId::RegisterField(
	TypeId classId, VariableId MemberId, 
	const std::string& fieldName, uint32_t Offset, 
	uint32_t Size, uint32_t Align, ExportInfo ExportInfo
)
{
	MemberInfo info{};
	info.Name = fieldName;
	info.VariableId = MemberId;
	info.Offset = Offset;
	info.Size = Size;
	info.Align = Align;
	info.ExportInfo = ExportInfo;

	GetStatics().MemberInfos[classId].emplace(info);
	GetStatics().MemberInfoNameMap[classId].emplace(fieldName, Offset);

	return { classId, Offset };
}

inline const MemberId::MemberInfo& MemberId::GetMemberInfo(TypeId id, const std::string& FieldName)
{
	auto& MemberInfoNames = GetStatics().MemberInfoNameMap[id];

	assert(MemberInfoNames.contains(FieldName));

	const MemberId MemberId{ id, MemberInfoNames[FieldName] };
	return MemberId.GetMemberInfo();
}

template<typename Class, typename Field>
inline MemberId MemberId::RegisterField(const std::string& fieldName, uint32_t Offset, ExportInfo ExportInfo)
{
	auto registerClass = RegisterType<Class>();
	auto registerField = RegisterType<Field>();

	return RegisterField(
		TypeId::Create<Class>(),
		VariableId::Create<Field>(),
		fieldName,
		Offset,
		sizeof(Field),
		alignof(Field),
		ExportInfo);
}

template<typename Field>
inline MemberId MemberId::RegisterField(TypeId classId, const std::string& fieldName, uint32_t Offset, ExportInfo ExportInfo)
{
	auto registerField = RegisterType<Field>();

	return RegisterField(
		classId,
		VariableId::Create<Field>(),
		fieldName,
		Offset,
		sizeof(Field),
		alignof(Field),
		ExportInfo);
}
