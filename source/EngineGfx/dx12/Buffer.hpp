
    inline void Buffer::copyData(const void* data, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ResourceState state)
    {
        m_uploadBuffer.init(device, 1, m_bufferSize, false);

        D3D12_SUBRESOURCE_DATA subresData = {};
        subresData.pData = data;
        subresData.RowPitch = m_bufferSize;
        subresData.SlicePitch = 1;

        transition(commandList, ResourceState::COPY_DEST);
        UpdateSubresources(commandList, resource(), m_uploadBuffer.resource(), 0, 0, 1, &subresData);
        transition(commandList, state);
    }

    template<typename T>
    void Buffer::init(GfxContext & context, const BufferDescription<T>& bufferDesc)
    {
        m_type = bufferDesc.type;
        m_elementSize = sizeof(T);
        m_bufferSize = bufferDesc.elementCount* m_elementSize;
        ResourceDescription desc{
                .format = DXGI_FORMAT_UNKNOWN,
                .width = m_bufferSize,
                .height = 1,
                .depthOrArraySize = 1,
                .dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .flags = ResourceFlags::NONE,
                .createState = ResourceState::COMMON,
                .heapType = D3D12_HEAP_TYPE_DEFAULT,
                .name = bufferDesc.name.data()
        };

        // TODO remake this thing
        DescriptorProperties descriptorProps = {
            .descriptor = DescriptorFlags::ShaderResource,
            .viewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .bufferStride = sizeof(T),
            .numElements = bufferDesc.elementCount
        };
        Resource::initResource(context.device, desc, descriptorProps);

        if(bufferDesc.data)
            copyData(bufferDesc.data, context.device, context.cmdList, bufferDesc.state);
    }
    template<typename T>
    Buffer::Buffer(GfxContext& context, T* data, uint numberOfElements, std::string_view name)
    {
        m_type = BufferType::CUSTOM;
        init(context, data, numberOfElements, name);
    }

    template<typename T>
    Buffer Buffer::CreateVertexBuffer(GfxContext& context, T* data, uint numberOfElements)
    {
        Buffer buffer;
        buffer.init(context, data, numberOfElements, "VertexBuffer", ResourceState::VERTEX_CONSTANT_BUFFER, BufferType::VERTEX);
        return buffer;
    }

    template<typename T>
    Buffer Buffer::CreateIndexBuffer(GfxContext& context, T* data, uint numberOfElements)
    {
        Buffer buffer;
        buffer.init(context, data, numberOfElements, "IndexBuffer", ResourceState::INDEX_BUFFER, BufferType::INDEX);
        return buffer;
    }

    inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(Buffer& buffer) {
        D3D12_INDEX_BUFFER_VIEW view;
        view.BufferLocation = buffer.resource()->GetGPUVirtualAddress();
        view.Format = DXGI_FORMAT_R32_UINT;
        view.SizeInBytes = buffer.getBufferSize();
        return view;
    }

    inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(Buffer& buffer) {
        D3D12_VERTEX_BUFFER_VIEW view;
        view.BufferLocation = buffer.resource()->GetGPUVirtualAddress();
        view.StrideInBytes = buffer.getElementSize();
        view.SizeInBytes = buffer.getBufferSize();
        return view;
    }
