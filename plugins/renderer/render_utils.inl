#pragma once

template<typename T>
inline T align_uniform_buffer_size(T size, T alignment = 64);

template<typename T>
inline T align_uniform_buffer_size(T size, T alignment) {
	return (size + alignment) & ~alignment - 1;
}