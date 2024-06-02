#pragma once

namespace engine
{
	namespace directx
	{
		void Init();
		void InitializePipeline();
		void InitializeAssets();
		void Update();
		void Render();
		void Destroy();
		void PopulateCommandList();
		void WaitForPreviousFrame();

	}
}