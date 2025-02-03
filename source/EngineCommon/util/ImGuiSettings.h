#pragma once
#include <d3d12.h>
#include <functional>
#include <string_view>
#include <vector>
#include "EngineCommon/include/types.h"
class BaseDemo;

namespace engine::util
{

	namespace imgui
	{
		void Init(HWND hwnd, ID3D12Device*, int numFrames);
		void StartFrame();
		void EndFrame(ID3D12GraphicsCommandList* cmdList);
		void Begin(std::string_view name);
		void End();
		bool SliderFloat(std::string_view name, float* ptr, float min, float max);
		bool SliderFloat2(std::string_view name, float* ptr, float min, float max);
		bool SliderFloat3(std::string_view name, float* ptr, float min, float max);
		bool SliderFloat4(std::string_view name, float* ptr, float min, float max);
		bool ColorEdit3(std::string_view label, float* col);
		bool Button(std::string_view label);

		struct FrameContext
		{
			ID3D12CommandAllocator* CommandAllocator;
			UINT64                  FenceValue;
		};
		static float clear_color[] = { 0.45f, 0.55f, 0.60f, 1.00f };
		extern ID3D12Device* g_pd3dDevice;
		extern ID3D12DescriptorHeap* g_pd3dSrvDescHeap;
		inline ID3D12DescriptorHeap* GetDescriptorHeap() {	return g_pd3dSrvDescHeap;	}
	};

	struct UI_Element
	{
		using uiActionCallback = std::function<void(BaseDemo&)>;
		static inline std::vector<UI_Element*> s_uiElements;

		virtual void onUIAction(BaseDemo&) = 0;
		uiActionCallback m_callback;
		UI_Element()
		{
			m_arrayIndex = s_uiElements.size();
			s_uiElements.push_back(this);
		}

		UI_Element(UI_Element&& other) : m_callback(other.m_callback), m_arrayIndex(other.m_arrayIndex)
		{
			other.m_arrayIndex = -1;
			s_uiElements[m_arrayIndex] = this;
		}

		~UI_Element()
		{
			if(m_arrayIndex != -1)
				s_uiElements.erase(s_uiElements.begin()+m_arrayIndex);
		}

		UI_Element operator=(const UI_Element& other) = delete;
		UI_Element(const UI_Element& other) = delete;

	private: 
		i32 m_arrayIndex = -1;
	};

	struct UI_Vector : public UI_Element
	{
		using slider = std::function<bool(std::string_view name, float* ptr, float min, float max)>;
		void onUIAction(BaseDemo& demo) override
		{
			if (m_function(m_name, m_ptr, -m_range, m_range))
				m_callback(demo);
		}
		
		UI_Vector(u32 dim, float* data, std::string_view name, float range)	:
			m_dim(dim), m_range(range), m_ptr(data), m_name(name)
		{
			if(m_dim == 3)
				m_function = imgui::SliderFloat3;
			else
				m_function = imgui::SliderFloat4;
		}

		u32 m_dim = 0 ;
		float m_range = 0;
		float* m_ptr = nullptr;
		std::string m_name;
		slider m_function;
	};

};
