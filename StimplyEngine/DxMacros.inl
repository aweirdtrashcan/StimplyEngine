#ifdef _DEBUG
#define DXERR(x, msg) if (GlobalContext::infoQueue) { GlobalContext::infoQueue->ClearStoredMessages(); GetError(); } if (FAILED(x)) { __debugbreak(); assert(false && msg); }
#else
#define DXERR(x, msg) (x)
#endif

#define GetError()\
{\
	ID3D11InfoQueue* __infoQueue = GlobalContext::infoQueue;\
	UINT64 numStoredMsg = __infoQueue->GetNumStoredMessages();\
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
			__infoQueue->GetMessageW(i, nullptr, &msgLength);\
			__infoQueue->GetMessageW(i, &message, &msgLength);\
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