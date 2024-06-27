#pragma once
/**
 * @addtogroup TYPES
 * @{
 */

#include <string>
#include <vector>

#include "eistype.h"

namespace eis
{

class EisSpectra
{
public:
	static constexpr int F_VERSION_MAJOR = 1;
	static constexpr int F_VERSION_MINOR = 1;
	static constexpr int F_VERSION_PATCH = 0;
	static constexpr char F_MAGIC[] = "EISF";

public:
	std::vector<DataPoint> data;
	std::string model;
	std::string headerDescription;
	std::string header;
	std::vector<double> labels;
	std::vector<std::string> labelNames;

public:
	/**
	 * @brief Constructs an EisSpectra.
	 *
	 * @param data Vector of the data points of the spectra.
	 * @param model Model description string for this spectra.
	 * @param header A free-form text that will be included in any save of this spectra.
	 * @param labels An optional vector of giving the values of the inputs of the model to get this spectra.
	 * @param labelNames An optional vector of names describing every input of the model.
	 */
	EisSpectra(const std::vector<DataPoint>& data, const std::string& model, const std::string& header,
			   std::vector<double> labels = std::vector<double>(),
			   std::vector<std::string> labelNames = std::vector<std::string>());

	/**
	 * @brief Constructs an EisSpectra.
	 *
	 * This function differs from the above only in the datatype of the label.
	 *
	 * @param data Vector of the data points of the spectra.
	 * @param model Model description string for this spectra.
	 * @param header A free-form text that will be included in any save of this spectra.
	 * @param labels An optional vector of giving the values of the inputs of the model to get this spectra.
	 * @param labelNames An optional vector of names describing every input of the model.
	 */
	EisSpectra(const std::vector<DataPoint>& data, const std::string& model, const std::string& header,
			   std::vector<float> labels, std::vector<std::string> labelNames = std::vector<std::string>());

	/**
	 * @brief Constructs an EisSpectra.
	 *
	 * This function differs from the above only in the datatype of the label.
	 *
	 * @param data Vector of the data points of the spectra.
	 * @param model Model description string for this spectra.
	 * @param header A free-from text that will be included in any save of this spectra.
	 * @param labels An optional vector of giving the values of the inputs of the model to get this spectra.
	 * @param labelNames An optional vector of names describing every input of the model.
	 */
	EisSpectra(const std::vector<DataPoint>& data, const std::string& model, const std::string& header,
			   std::vector<size_t> labels, std::vector<std::string> labelNames = std::vector<std::string>());

	/**
	 * @brief Constructs an EisSpectra.
	 *
	 * This function differs from the above only in the datatype of the label.
	 *
	 * @param data Vector of the data points of the spectra.
	 * @param model Model description string for this spectra.
	 * @param header A free-form text that will be included in any save of this spectra.
	 * @param label A value corresponding to all inputs of the model.
	 * @param maxLabel The number of inputs of the model.
	 */
	EisSpectra(const std::vector<DataPoint>& data, const std::string& model, const std::string& header,
			   size_t label, size_t maxLabel, std::vector<std::string> labelNames = std::vector<std::string>());

	/**
	 * @brief Constructs a EisSpectra by loading an EIS file from disk.
	 *
	 * @throw eis::file_error if there is an error loading the file
	 * @param path The path to the file.
	 */
	EisSpectra(const std::filesystem::path& path){*this = loadFromDisk(path);}

	EisSpectra(){}

	/**
	 * @brief Constructs a EisSpectra by loading a EIS file from disk.
	 *
	 * This function has the attribute [[nodiscard]]
	 *
	 * @throw eis::file_error if there is an error loading the file
	 * @param path The path to the file.
	 * @return The EisSpectra parsed from the file.
	 */
	[[nodiscard]] static EisSpectra loadFromDisk(const std::filesystem::path& path);

	/**
	 * @brief Constructs a EisSpectra by loading a EIS file from a stream.
	 *
	 * This function has the attribute [[nodiscard]]
	 *
	 * @throw eis::file_error if there is an error loading the file
	 * @param stream The stream that contains the EIS file.
	 * @return The EisSpectra parsed from the stream.
	 */
	[[nodiscard]] static EisSpectra loadFromStream(std::istream& stream);

	/**
	 * @brief Sets all input values up to a maximum given by maxLabel to the value given by label.
	 *
	 * @param label The value to apply to all inputs.
	 * @param maxLabel The maximum number of values to set.
	 */
	void setLabel(size_t label, size_t maxLabel);

	/**
	 * @brief Gets the input value of this model, where it is a single value.
	 *
	 * @return The input value,
	 */
	size_t getLabel();

	/**
	 * @brief Sets the input values of this model.
	 *
	 * @param label The input values.
	 */
	void setSzLabels(std::vector<size_t> label);

	/**
	 * @brief Sets the input values of this model.
	 *
	 * @param label The input values.
	 */
	void setLabels(const std::vector<double>& labelsIn);

	/**
	 * @brief Sets the input values of this model.
	 *
	 * @param label The input values.
	 */
	void setLabels(const std::vector<float>& labelsIn);

	/**
	 * @brief Sets the input values of this model.
	 *
	 * @param label The input values.
	 */
	std::vector<size_t> getSzLabels() const;

	/**
	 * @brief Returns true if there are multiple inputs, false otherwise.
	 *
	 * @return true if there are multiple inputs, false otherwise.
	 */
	bool isMulticlass();

	/**
	 * @brief Returns the inputs as a vector.
	 *
	 * @return The inputs as a vector.
	 */
	std::vector<fvalue> getFvalueLabels();

	/**
	 * @brief Saves the spectra to disk.
	 *
	 * @param path A path to the file on disk where the spectra shall be saved.
	 * @return true on success, false on failure.
	 */
	bool saveToDisk(const std::filesystem::path& path) const;

	/**
	 * @brief Saves the spectra in the given stream.
	 *
	 * @param stream A std::ostream into which the spectra will be saved.
	 */
	void saveToStream(std::ostream& stream) const;
};

/**
 * @brief Deprecated function use eis::EisSpectra::saveToDisk instead.
 */
[[deprecated]] bool saveToDisk(const EisSpectra& data, const std::filesystem::path& path);

/**
 * @brief Deprecated function use eis::EisSpectra::loadFromDisk instead.
 */
[[deprecated]] [[nodiscard]] EisSpectra loadFromDisk(const std::filesystem::path& path);

}

std::ostream &operator<<(std::ostream &s, eis::EisSpectra const& spectra);

/** @} */
