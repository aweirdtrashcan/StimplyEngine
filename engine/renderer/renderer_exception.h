#include <exception>

class RendererException : public std::exception {
public:
    RendererException(const char* error, ...);
    virtual ~RendererException();

    virtual const char* what() const noexcept { return m_What; }

private:
    char* m_What;
};