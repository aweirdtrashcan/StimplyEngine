#pragma once

class Mesh {
	friend class MeshLoader;
public:
	Mesh();
	~Mesh();

	bool IsLoaded() const;

private:
	void CopyName(char* name);
private:
	char* name;
};