#pragma once
#include <cassert>


namespace Piccolo
{
	template <typename T>
	struct Slice1D
	{
		int size;
		T* data;

		Slice1D(int _size, T* _data) : size(_size), data(_data)
		{
		}

		void zero() { memset((char*)data, 0, sizeof(T) * size); }

		void set(const T& x)
		{
			for (int i = 0; i < size; i++)
			{
				data[i] = x;
			}
		}

		inline T& operator()(int i) const
		{
			assert(i >= 0 && i < size);
			return data[i];
		}
	};

	// Same but for a 2d array of data.
	template <typename T>
	struct Slice2D
	{
		int rows, cols;
		T* data;

		Slice2D(int _rows, int _cols, T* _data) : rows(_rows), cols(_cols), data(_data)
		{
		}

		void zero() { memset((char*)data, 0, sizeof(T) * rows * cols); }

		void set(const T& x)
		{
			for (int i = 0; i < rows * cols; i++)
			{
				data[i] = x;
			}
		}

		inline Slice1D<T> operator()(int i) const
		{
			assert(i >= 0 && i < rows);
			return Slice1D<T>(cols, &data[i * cols]);
		}

		inline T& operator()(int i, int j) const
		{
			assert(i >= 0 && i < rows && j >= 0 && j < cols);
			return data[i * cols + j];
		}
	};

	//--------------------------------------

	// These types are used for the storage of arrays of data.
	// They implicitly cast to slices so can be given directly
	// as inputs to functions requiring them.
	template <typename T>
	struct Array1D
	{
		int size;
		T* data;

		Array1D() : size(0), data(NULL)
		{
		}

		Array1D(int _size) : Array1D() { resize(_size); }

		Array1D(const Slice1D<T>& rhs) : Array1D()
		{
			resize(rhs.size);
			memcpy(data, rhs.data, rhs.size * sizeof(T));
		}

		Array1D(const Array1D<T>& rhs) : Array1D()
		{
			resize(rhs.size);
			memcpy(data, rhs.data, rhs.size * sizeof(T));
		}

		~Array1D() { resize(0); }

		Array1D& operator=(const Slice1D<T>& rhs)
		{
			resize(rhs.size);
			memcpy(data, rhs.data, rhs.size * sizeof(T));
			return *this;
		};

		Array1D& operator=(const Array1D<T>& rhs)
		{
			resize(rhs.size);
			memcpy(data, rhs.data, rhs.size * sizeof(T));
			return *this;
		};

		inline T& operator()(int i) const
		{
			assert(i >= 0 && i < size);
			return data[i];
		}

		operator Slice1D<T>() const { return Slice1D<T>(size, data); }

		void zero() { memset(data, 0, sizeof(T) * size); }

		void set(const T& x)
		{
			for (int i = 0; i < size; i++)
			{
				data[i] = x;
			}
		}

		void resize(int _size)
		{
			if (_size == 0 && size != 0)
			{
				free(data);
				data = NULL;
				size = 0;
			}
			else if (_size > 0 && size == 0)
			{
				data = (T*)malloc(_size * sizeof(T));
				size = _size;
				assert(data != NULL);
			}
			else if (_size > 0 && size > 0 && _size != size)
			{
				data = (T*)realloc(data, _size * sizeof(T));
				size = _size;
				assert(data != NULL);
			}
		}
	};

	template <typename T>
	void array1d_write(const Array1D<T>& arr, FILE* f)
	{
		fwrite(&arr.size, sizeof(int), 1, f);
		size_t num = fwrite(arr.data, sizeof(T), arr.size, f);
		assert((int)num == arr.size);
	}

	template <typename T>
	void array1d_read(Array1D<T>& arr, FILE* f)
	{
		int size;
		fread(&size, sizeof(int), 1, f);
		arr.resize(size);
		size_t num = fread(arr.data, sizeof(T), size, f);
		assert((int)num == size);
	}

	// Similar type but for 2d data
	template <typename T>
	struct Array2D
	{
		int m_rows, m_cols;
		T* data;

		Array2D() : m_rows(0), m_cols(0), data(NULL)
		{
		}

		Array2D(int _rows, int _cols) : Array2D() { resize(_rows, _cols); }
		~Array2D() { resize(0, 0); }

		Array2D& operator=(const Array2D<T>& rhs)
		{
			resize(rhs.m_rows, rhs.m_cols);
			memcpy(data, rhs.data, rhs.m_rows * rhs.m_cols * sizeof(T));
			return *this;
		};

		Array2D& operator=(const Slice2D<T>& rhs)
		{
			resize(rhs.m_rows, rhs.m_cols);
			memcpy(data, rhs.data, rhs.m_rows * rhs.m_cols * sizeof(T));
			return *this;
		};

		inline Slice1D<T> operator()(int i) const
		{
			assert(i >= 0 && i < m_rows);
			return Slice1D<T>(m_cols, &data[i * m_cols]);
		}

		inline T& operator()(int i, int j) const
		{
			assert(i >= 0 && i < m_rows && j >= 0 && j < m_cols);
			return data[i * m_cols + j];
		}

		operator Slice2D<T>() const { return Slice2D<T>(m_rows, m_cols, data); }

		void zero() { memset(data, 0, sizeof(T) * m_rows * m_cols); }

		void set(const T& x)
		{
			for (int i = 0; i < m_rows * m_cols; i++)
			{
				data[i] = x;
			}
		}

		void resize(int _rows, int _cols)
		{
			int _size = _rows * _cols;
			int size = m_rows * m_cols;

			if (_size == 0 && size != 0)
			{
				free(data);
				data = NULL;
				m_rows = 0;
				m_cols = 0;
			}
			else if (_size > 0 && size == 0)
			{
				data = (T*)malloc(_size * sizeof(T));
				m_rows = _rows;
				m_cols = _cols;
				assert(data != NULL);
			}
			else if (_size > 0 && size > 0 && _size != size)
			{
				data = (T*)realloc(data, _size * sizeof(T));
				m_rows = _rows;
				m_cols = _cols;
				assert(data != NULL);
			}
		}
	};

	template <typename T>
	void array2d_write(const Array2D<T>& arr, FILE* f)
	{
		fwrite(&arr.m_rows, sizeof(int), 1, f);
		fwrite(&arr.m_cols, sizeof(int), 1, f);
		size_t num = fwrite(arr.data, sizeof(T), arr.m_rows * arr.m_cols, f);
		assert((int)num == arr.m_rows * arr.m_cols);
	}

	template <typename T>
	void array2d_read(Array2D<T>& arr, FILE* f)
	{
		int rows, cols;
		auto r = fread(&rows, sizeof(int), 1, f);
        r = fread(&cols, sizeof(int), 1, f);
		arr.resize(rows, cols);
		size_t num = fread(arr.data, sizeof(T), rows * cols, f);
		assert((int)num == rows * cols);
	}

}
