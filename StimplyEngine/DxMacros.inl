#ifdef _DEBUG
#define DXERR(x, msg) if (m_InfoQueue) { m_InfoQueue->ClearStoredMessages(); GetError(); } if (FAILED(x)) { __debugbreak(); assert(false && msg); }
#else
#define DXERR(x, msg) (x)
#endif

#define SetupBindableInfo()\
	if (deviceCtx != nullptr && deviceCtx != m_DeviceCtx) m_DeviceCtx = deviceCtx

#define SetupDrawableInfo()\
	if (deviceCtx != nullptr && deviceCtx != m_DeviceCtx) m_DeviceCtx = deviceCtx;

#define GetError()\
{\
	UINT64 numStoredMsg = m_InfoQueue->GetNumStoredMessages();\
\
	std::stringstream ss;\
\
	std::string str;\
	const char* strChar = 0;\
\
	if (numStoredMsg)\
	{\
		for (UINT64 i = 0; i < numStoredMsg; ++i)\
		{\
			D3D11_MESSAGE message{};\
			SIZE_T msgLength = 0;\
			m_InfoQueue->GetMessageW(i, nullptr, &msgLength);\
			m_InfoQueue->GetMessageW(i, &message, &msgLength);\
\
			ss << message.pDescription << std::endl;\
		}\
\
		str = ss.str();\
		strChar = str.c_str();\
\
		MessageBoxA(nullptr, strChar, "Fatal error", MB_OK | MB_ICONEXCLAMATION);\
	}\
}

#define SetupDebugger()\
	Microsoft::WRL::ComPtr<ID3D11InfoQueue> m_InfoQueue = m_DeviceCtx->infoQueue.Get();

#define InfoMan() SetupBindableInfo(); SetupDebugger();