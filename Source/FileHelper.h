#pragma once

#include "Prefix.h"

template <typename T, typename = void>
struct HasCustomSerialization : std::false_type {};

template <typename T>
struct HasCustomSerialization<T, std::void_t<decltype(std::declval<T>().Serialize(std::declval<std::fstream&>()))>> : std::true_type {};

template<typename T>
static constexpr bool HasCustomSerializationValue = HasCustomSerialization<T>::value;


template<typename T> struct SerializeHelper
{
	void operator()(std::fstream& FileStream, const T& Object)
	{
		if constexpr (HasCustomSerializationValue<T>)
		{
			Object.Serialize(FileStream);
		}
		else
		{
			FileStream.write((char*)(&Object), sizeof(T));
		}
	}
};

template<typename T> struct DeserializeHelper
{
	void operator()(std::fstream& FileStream, T& Object)
	{
		if constexpr (HasCustomSerializationValue<T>)
		{
			Object.Deserialize(FileStream);
		}
		else
		{
			FileStream.read((char*)(&Object), sizeof(T));
		}
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
			SerializeHelper<T>()(FileStream, Object[i]);
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
			DeserializeHelper<T>()(FileStream, Object[i]);
		}
	}
};


template<>
struct SerializeHelper<std::string>
{
	void operator()(std::fstream& FileStream, const std::string& str)
	{
		size_t Count = str.length();
		FileStream.write((char*)(&Count), sizeof(Count));
		FileStream.write((char*)(&str[0]), sizeof(char) * Count);
	}
};

template<>
struct DeserializeHelper<std::string>
{
	void operator()(std::fstream& FileStream, std::string& Object)
	{
		size_t Count;
		FileStream.read((char*)(&Count), sizeof(Count));
		Object.resize(Count);
		FileStream.read((char*)(&Object[0]), sizeof(char) * Count);
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

	void ReadAllBinaryFile(std::vector<char>& buffer)
	{
		FileStream.seekg(0, std::ios::end);
		auto BytesSize = FileStream.tellg();
		FileStream.seekg(0, std::ios::beg);
		buffer.resize(BytesSize);
		FileStream.read(buffer.data(), BytesSize);
	}

public:
	std::fstream FileStream;
};
