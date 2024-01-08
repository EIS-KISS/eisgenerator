#pragma once
#include <complex>
#include <vector>
#include <valarray>
#include <cassert>
#include <cmath>
#include <filesystem>

/**
 * @brief Datatype for all calculations
 */
typedef float fvalue;

namespace eis
{

/**
* Types used by eisgenerator
* @defgroup TYPES Types
* @{
*/

/**
* @brief Basic singluar EIS data point
*/
class DataPoint
{
public:
	std::complex<fvalue> im; /**< Complex impedance */
	fvalue omega; /**< Frequency of the complex impedance */
	DataPoint() = default;
	DataPoint(std::complex<fvalue> imIn, fvalue omegaIn = 0): im(imIn), omega(omegaIn){}
	bool operator<(const DataPoint& in) const
	{
		return omega < in.omega;
	}
	bool operator>(const DataPoint& in) const
	{
		return omega > in.omega;
	}
	bool operator==(const DataPoint& in) const
	{
		return im == in.im;
	}
	DataPoint operator-(const DataPoint& in) const
	{
		DataPoint out(*this);
		out.im = out.im - in.im;
		return out;
	}
	DataPoint operator+(const DataPoint& in) const
	{
		DataPoint out(*this);
		out.im = out.im + in.im;
		return out;
	}
	DataPoint operator/(fvalue in) const
	{
		DataPoint out(*this);
		out.im = out.im / in;
		return out;
	}
	DataPoint operator*(fvalue in) const
	{
		DataPoint out(*this);
		out.im = out.im * in;
		return out;
	}
	DataPoint operator/=(fvalue in)
	{
		im = im / in;
		return *this;
	}
	/**
	* @brief calculates the absolute value of the complex impedance
	*
	* @return absolute value of the complex impedance
	*/
	fvalue complexVectorLength() const
	{
		return std::sqrt(std::pow(im.real(), 2) + std::pow(im.imag(), 2));
	}
};


/**
* @brief A range
*/
class Range
{
public:
	fvalue start; /**< Start of the range*/
	fvalue end; /**< End of the range*/
	size_t count = 0; /**< Numer of elements in the range*/
	size_t step = 0; /**< Currently active step*/
	bool log = false; /**< True if the elements in the range are to be spaced in log10 increments*/

	/**
	* @brief calculats the distance between elements in the range
	* will calcuate the log10 of the distance if the range is logarithmic
	*
	* @return the distance between steps of this range
	*/
	fvalue stepSize() const
	{
		fvalue startL = log ? log10(start) : start;
		fvalue endL = log ? log10(end) : end;
		return (endL-startL)/(count-1);
	}

	/**
	* @brief calucates the value of the current step
	*
	* @return the value of the current step
	*/
	fvalue stepValue() const
	{
		return at(step);
	}

	/**
	* @brief calucates the mean of the start and the end values
	*
	* @return the mean of the start and the end values
	*/
	fvalue center() const
	{
		return (start+end)/2;
	}

	/**
	* @brief calucates the value at the given index
	*
	* @return the value at the given index
	*/
	fvalue at(size_t index) const
	{
		assert(index < count || (index == 0 && count == 0));
		if(count < 2)
			return start;
		return log ? pow(10, stepSize()*index+log10(start)) : stepSize()*index+start;
	}

	fvalue operator[](size_t index) const
	{
		return at(index);
	}
	Range operator*(fvalue in) const
	{
		return Range(start*in, end*in, count, log);
	}
	Range operator/(fvalue in) const
	{
		return operator*(static_cast<fvalue>(1.0)/in);
	}
	Range operator*(int in) const
	{
		return operator*(static_cast<fvalue>(in));
	}
	Range operator/(int in) const
	{
		return operator*(static_cast<fvalue>(1.0)/in);
	}
	Range(fvalue startI, fvalue endI, size_t countI, bool logI = false): start(startI), end(endI), count(countI), log(logI){}
	Range() = default;

	/**
	* @brief prints the range to stdout via eis::Log
	*
	* @param level the eis::Log::Level to print at
	*/
	void print(int level) const;

	/**
	* @brief gets a machine parsable string encodeing this range
	*
	* @return the string
	*/
	std::string getString() const;

	/**
	* @brief checks if the values of this range are sane
	* this checks for some common errors like haveing a end < beign
	*
	* @return true if sane, false if not
	*/
	bool isSane() const;

	/**
	* @brief this function consturcts a vector that contains all elements of this range
	*
	* @return the vector with the elments of this range
	*/
	std::vector<fvalue> getRangeVector() const;

	/**
	* @brief this function creates a range from the parsable string
	*
	* @throw std::invalid_argument when the string syntax is incorrect
	* @return the range that was parsed from the string
	*/
	[[nodiscard]] static Range fromString(std::string str, size_t count);

	/**
	* @brief this function creates a vector ranges from the parsable parameter array string
	*
	* @throw std::invalid_argument when the string syntax is incorrect
	* @return the vector of ranges that was parsed from the string
	*/
	[[nodiscard]] static std::vector<Range> rangesFromParamString(const std::string& paramStr, size_t count);
};

class parse_errror: public std::exception
{
	std::string whatStr;
public:
	parse_errror(const std::string& whatIn): whatStr(whatIn)
	{}
	virtual const char* what() const noexcept override
	{
		return whatStr.c_str();
	}
};

class file_error: public std::exception
{
	std::string whatStr;
public:
	file_error(const std::string& whatIn): whatStr(whatIn)
	{}
	virtual const char* what() const noexcept override
	{
		return whatStr.c_str();
	}
};

class EisSpectra
{
public:
	static constexpr int F_VERSION_MAJOR = 1;
	static constexpr int F_VERSION_MINOR = 0;
	static constexpr int F_VERSION_PATCH = 0;
	static constexpr char F_MAGIC[] = "EISF";

public:
	std::vector<DataPoint> data;
	std::string model;
	std::string header;
	std::vector<double> labels;
	std::vector<std::string> labelNames;

public:
	/**
	* @brief Constructs a EisSpectra
	*
	* @param data vector of the data points of the spectra
	* @param model model descritpion string for this spectra
	* @param header a freform text that will be inclded in any save of this spectra
	* @param labels an optional vector of giving the values of the inputs of the model to get this spectra
	* @param labelNames an optional vector of names describing every input of the model
	*/
	EisSpectra(const std::vector<DataPoint>& data, const std::string& model, const std::string& header,
			   std::vector<double> labels = std::vector<double>(),
			   std::vector<std::string> labelNames = std::vector<std::string>());

	/**
	* @brief Constructs a EisSpectra
	* this function differes from the above only in the datatype of the label
	*
	* @param data vector of the data points of the spectra
	* @param model model descritpion string for this spectra
	* @param header a freform text that will be inclded in any save of this spectra
	* @param labels an optional vector of giving the values of the inputs of the model to get this spectra
	* @param labelNames an optional vector of names describing every input of the model
	*/
	EisSpectra(const std::vector<DataPoint>& data, const std::string& model, const std::string& header,
			   std::vector<float> labels, std::vector<std::string> labelNames = std::vector<std::string>());

	/**
	* @brief Constructs a EisSpectra
	* this function differes from the above only in the datatype of the label
	*
	* @param data vector of the data points of the spectra
	* @param model model descritpion string for this spectra
	* @param header a freform text that will be inclded in any save of this spectra
	* @param labels an optional vector of giving the values of the inputs of the model to get this spectra
	* @param labelNames an optional vector of names describing every input of the model
	*/
	EisSpectra(const std::vector<DataPoint>& data, const std::string& model, const std::string& header,
			   std::vector<size_t> labels, std::vector<std::string> labelNames = std::vector<std::string>());

	/**
	* @brief Constructs a EisSpectra
	* this function differes from the above only in the datatype of the label
	*
	* @param data vector of the data points of the spectra
	* @param model model descritpion string for this spectra
	* @param header a freeform text that will be inclded in any save of this spectra
	* @param label a value corrisponding to all inputs of the model
	* @param maxLabel the number of inputs of the model
	*/
	EisSpectra(const std::vector<DataPoint>& data, const std::string& model, const std::string& header,
			   size_t label, size_t maxLabel, std::vector<std::string> labelNames = std::vector<std::string>());

	/**
	* @brief Constructs a EisSpectra by loading a EIS file from disk
	*
	* @throw eis::file_error if there is an error loading the file
	* @param path the path to the file
	*/
	EisSpectra(const std::filesystem::path& path){*this = loadFromDisk(path);}

	EisSpectra(){}

	/**
	* @brief Constructs a EisSpectra by loading a EIS file from disk
	*
	* @throw eis::file_error if there is an error loading the file
	* @param path the path to the file
	* @return the EisSpectra
	*/
	[[nodiscard]] static EisSpectra loadFromDisk(const std::filesystem::path& path);

	/**
	* @brief Constructs a EisSpectra by loading a EIS file from a stream
	*
	* @throw eis::file_error if there is an error loading the file
	* @param stream the stream that contains the EIS file
	* @return the EisSpectra
	*/
	[[nodiscard]] static EisSpectra loadFromStream(std::istream& stream);

	/**
	* @brief Constructs a EisSpectra by loading a EIS file from a stream
	*
	* @param stream the stream that contains the EIS file
	* @return the EisSpectra
	*/
	void setLabel(size_t label, size_t maxLabel);

	/**
	* @brief Gets the input value of this model, where it is a single value
	*
	* @return the input value
	*/
	size_t getLabel();

	/**
	* @brief Sets the input values of this model
	*
	* @param label the input values
	*/
	void setSzLabels(std::vector<size_t> label);

	/**
	* @brief Sets the input values of this model
	*
	* @param label the input values
	*/
	void setLabels(const std::vector<double>& labelsIn);

	/**
	* @brief Sets the input values of this model
	*
	* @param label the input values
	*/
	void setLabels(const std::vector<float>& labelsIn);

	/**
	* @brief Sets the input values of this model
	*
	* @param label the input values
	*/
	std::vector<size_t> getSzLabels() const;

	/**
	* @brief Returns true if there are multiple inputs, false otherwise
	*
	* @return true if there are multiple inputs, false otherwise
	*/
	bool isMulticlass();

	/**
	* @brief Returns the inputs as a vector
	*
	* @return the inputs as a vector
	*/
	std::vector<fvalue> getFvalueLabels();

	/**
	* @brief Saves the spectra to disk
	*
	* @param path a path to the file on disk where the spectra shal be saved
	* @return true on sucess, false on failure
	*/
	bool saveToDisk(const std::filesystem::path& path) const;

	/**
	 * @brief Saves the spectra in the given stream
	 *
	 * @param stream a std::ostream into which the specta will be saved
	 */
	void saveToStream(std::ostream& stream) const;
};

/**
 * @brief deprecated function use eis::EisSpectra::saveToDisk instead
 */
[[deprecated]] bool saveToDisk(const EisSpectra& data, const std::filesystem::path& path);

/**
 * @brief deprecated function use eis::EisSpectra::loadFromDisk instead
 */
[[deprecated]] [[nodiscard]] EisSpectra loadFromDisk(const std::filesystem::path& path);

/**
* @brief Returns the a vector of DataPoints as a pair of valarrays
*
* @return a pair of valarrays first the real part and second the imaginary part
*/
std::pair<std::valarray<fvalue>, std::valarray<fvalue>> eisToValarrays(const std::vector<eis::DataPoint>& b);

/**
* @brief Returns the mean l2 element wise distance of he given spectra
*
* @param a the first set of points
* @param b the second set of points, must be the same length as a
* @return the mean l2 distance
*/
fvalue eisDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b);


/**
* @brief Returns the mean distance of the points in a to the linearly interporlated nyquist curve of b
*
* This implementation is quite slow
*
* @param a the first set of points
* @param b the second set of points
* @return the mean nyquist distance
*/
fvalue eisNyquistDistance(const std::vector<eis::DataPoint>& a, const std::vector<eis::DataPoint>& b);

/** @} */

}

std::ostream &operator<<(std::ostream &s, eis::DataPoint const& dp);

std::ostream &operator<<(std::ostream &s, eis::Range const& range);

std::ostream &operator<<(std::ostream &s, eis::EisSpectra const& spectra);

