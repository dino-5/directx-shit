#pragma once
#include <d3d12.h>
#include <functional>
#include <string_view>
#include <vector>
#include "EngineCommon/include/types.h"
#include "EngineCommon/math/Vector.h"
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
    bool checkBox(std::string_view name, bool* value);

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

struct UI_ElementInterface
{
    static inline std::vector<UI_ElementInterface*> s_uiElements;

    virtual void onUIAction() = 0;
    UI_ElementInterface()
    {
        arrayIndex = (i32)s_uiElements.size();
        s_uiElements.push_back(this);
    }

    UI_ElementInterface(UI_ElementInterface&& other) : arrayIndex(other.arrayIndex)
    {
        other.arrayIndex = -1;
        s_uiElements[arrayIndex] = this;
    }

    virtual ~UI_ElementInterface()
    {
        if(arrayIndex != -1)
            s_uiElements.erase(s_uiElements.begin()+arrayIndex);
    }

    UI_ElementInterface operator=(const UI_ElementInterface& other) = delete;
    UI_ElementInterface(const UI_ElementInterface& other) = delete;

private: 
    i32 arrayIndex = -1;
};

template<typename Type, typename Function, typename Data>
struct UI_ElementGenericInterface : public UI_ElementInterface
{
    using uiActionCallback = std::function<void(Type&, Data)>;

    void onUIAction() override {}
    UI_ElementGenericInterface(Type& aObj, Data aData, std::string_view aName) : UI_ElementInterface(), object(aObj), data(aData), name(aName) {}

    UI_ElementGenericInterface(UI_ElementGenericInterface&& other) : UI_ElementInterface(other), callback(other.callback), object(other.object)
    {
        other.object = nullptr;
    }

    Data getData()const { return data; }
    void setCallback(uiActionCallback aCallback) { callback = aCallback; }
protected:
    void setFunction(Function f) { function = f; }
    Type& getObject() { return object; }
    Data* getDataPtr(){ return &data; }
    std::string_view getStringView() { return name; }

    Type& object;
    uiActionCallback callback;
    Data data;
    Function function;
    std::string name;
};

#define superFunctions() \
protected:\
    using Super::getDataPtr; \
    using Super::getObject; \
    using Super::setFunction; \
    using Super::function; \
    using Super::getStringView; \
    using Super::callback; \
public:\
    using Super::getData;   \


using Slider = std::function<bool(std::string_view name, float* ptr, float min, float max)>;
using CheckBox = std::function<bool(std::string_view name, bool* ptr)>;

template<typename T>
struct UI_Float : public UI_ElementGenericInterface<T, Slider, float>
{
    using Super = UI_ElementGenericInterface<T, Slider, float>;
    superFunctions()
    void onUIAction() override
    {
        if (function(getStringView(), getDataPtr(), -range, range) && callback)
            callback(getObject(), getData());
    }
    
    UI_Float(T& aObject, float aData, std::string_view aName, float aRange)	:
        Super(aObject, aData, aName), range(aRange)
    {
        setFunction(imgui::SliderFloat);
    }

public:
    float range = 0;
};

template<typename T, int D = 3>
struct UI_Vector : public UI_ElementGenericInterface<T, Slider, engine::math::Vector<D>>
{
    using Super = UI_ElementGenericInterface<T, Slider, math::Vector<D>>;
    superFunctions()
    void onUIAction() override
    {
        if (function(getStringView(), getDataPtr(), -range, range) && callback)
            callback(getObject(), getData());
    }
    
    UI_Vector(T& obj, engine::math::Vector<D> aData, std::string_view aName, float aRange)	:
        Super(obj, aData, aName), range(aRange)
    {
        if constexpr(D == 3)
            setFunction(imgui::SliderFloat3);
        else
            setFunction(imgui::SliderFloat4);
    }

    float range = 0;
};

template<typename T>
struct UI_CheckBox : public UI_ElementGenericInterface<T, CheckBox, bool>
{
    using Super = UI_ElementGenericInterface<T, CheckBox, bool>;
    superFunctions()
    void onUIAction() override
    {
        if (function(getStringView(), getDataPtr()) && callback)
            callback(getObject(), getData());
    }

    UI_CheckBox(T& obj, bool aValue, std::string_view aName) : Super(obj, aValue, aName)
    {
        setFunction(imgui::checkBox);
    }

};

};
