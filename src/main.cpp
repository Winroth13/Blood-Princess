#include <iostream>

#include "core/application.hpp"

#include "reflection/MemberVars.h"
#include "components/transform_component.hpp"

// Hint to make the application use dedicated graphics cards (NIVIDA/AMD)
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
}

int main()
{
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	TypeId id = TypeId::Create<TransformComponent>();
	std::cout << id.GetTypeName() << " contains:\n";
	std::cout << id.GetTypeSize() << "\n";
	for (auto &member : MemberId::GetMemberInfos(id))
	{
		std::cout << '\t' << GetVariableName(member.VariableId) << '[' << member.Name << "]: Size [" << member.Size << "] align [" << member.Align << "] offset [" << member.Offset <<"]\n";
	}

	Application app;

	if (!app.Initialize()) 
		return 1;

	app.Run();
	app.Shutdown();

	return 0;
}
