#pragma once

#include "Prefix.h"

template<typename T> struct SerializeHelper
{
	void operator()(std::fstream& FileStream, const T& Object)
	{
		FileStream.write((char*)(&Object), sizeof(T));
	}
};

template<typename T> struct DeserializeHelper 
{
	void operator()(std::fstream& FileStream, T& Object)
	{
		FileStream.read((char*)(&Object), sizeof(T));
	}
};

template<typename T>
struct SerializeHelper<std::vector<T>>
{
	void operator()(std::fstream& FileStream, const std::vector<T>& Object)
	{
		size_t Count = Object.size();
		FileStream.write((char*)(&Count), sizeof(Count));
		for (size_t i = 0; i < Count; i++)
		{
			FileStream.write((char*)(&Object[i]), sizeof(T));
		}
	}
};

template<typename T> 
struct DeserializeHelper<std::vector<T>>
{
	void operator()(std::fstream& FileStream, std::vector<T>& Object)
	{
		size_t Count;
		FileStream.read((char*)(&Count), sizeof(Count));
		Object.resize(Count);
		for (size_t i = 0; i < Count; i++)
		{
			FileStream.read((char*)(&Object[i]), sizeof(T));
		}
	}
};

class FileHelper
{
public:
	enum FileMode
	{
		FileRead,
		FileWrite
	};

public:
	FileHelper(const std::string& File, FileMode Mode)
	{
		if (Mode == FileMode::FileRead)
			FileStream.open(File.c_str(), std::ios::in | std::ios::binary);
		else if (Mode == FileMode::FileWrite)
			FileStream.open(File.c_str(), std::ios::out | std::ios::binary);
		assert(FileStream);
	}

	~FileHelper()
	{
		FileStream.close();
	}

public:
	template<typename T> void Write(const T& Object)
	{
		SerializeHelper<T> helper;
		helper(FileStream, Object);
	}

	template<typename T>
	void Read(T& Object)
	{
		DeserializeHelper<T> helper;
		helper(FileStream, Object);
	}

public:
	std::fstream FileStream;
};
