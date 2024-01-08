#include <iostream>
#include <eisgenerator/model.h>
#include <eisgenerator/log.h>
#include <torch/torch.h>

template <typename V>
inline torch::TensorOptions tensorOptCpu(bool grad = true)
{
	static_assert(std::is_same<V, float>::value || std::is_same<V, double>::value,
				  "This function can only be passed double or float types");
	torch::TensorOptions options;
	if constexpr(std::is_same<V, float>::value)
		options = options.dtype(torch::kFloat32);
	else
		options = options.dtype(torch::kFloat64);
	options = options.layout(torch::kStrided);
	options = options.device(torch::kCPU);
	options = options.requires_grad(grad);
	return options;
}

torch::Tensor fvalueVectorToTensor(std::vector<fvalue>& vect)
{
	return torch::from_blob(vect.data(), {static_cast<int64_t>(vect.size())}, tensorOptCpu<fvalue>());
}

std::shared_ptr<torch::CompilationUnit> compileModel(eis::Model &model)
{
	std::string torchScript = model.getTorchScript();
	std::shared_ptr<torch::CompilationUnit> compiledModule = torch::jit::compile(torchScript);
	return compiledModule;
}

std::shared_ptr<torch::CompilationUnit> compileModel(std::string modelstr)
{
	eis::Model model(modelstr);
	return compileModel(model);
}

torch::Tensor runScriptModel(eis::Model& model, std::shared_ptr<torch::CompilationUnit> compiledScript,
                             torch::Tensor parameters, torch::Tensor omegas)
{
	torch::Tensor result = compiledScript->run_method(model.getCompiledFunctionName(), parameters, omegas).toTensor();
	assert(result.scalar_type() == torch::kComplexFloat || result.scalar_type() == torch::kComplexDouble);
	assert(result.size(0) == omegas.numel());
	assert(result.sizes().size() == 1 || result.size(1) == 1);
	if(result.sizes().size() == 2)
		result = result.reshape({result.numel()});
	return result;
}

torch::Tensor runScriptModel(eis::Model& model, size_t step,
                             std::shared_ptr<torch::CompilationUnit> compiledScript,
                             torch::Tensor omegas, torch::Device device)
{
	model.resolveSteps(step);
	std::vector<fvalue> parameterVec = model.getFlatParameters();
	torch::Tensor parameters = fvalueVectorToTensor(parameterVec).to(device);
	return runScriptModel(model, compiledScript, parameters, omegas);
}

int main(int argc, char** argv)
{
	// Set the verbosity of libeisgenerator
	eis::Log::level = eis::Log::WARN;

	std::string deviceString = torch::cuda::is_available() ? "cuda:0" : "cpu";
	std::cout<<"Using "<<deviceString<<" as compute device\n";
	torch::Device device(deviceString);

	// Create a model object from a circuit string
	eis::Model model("r{10~100}-r{10-1000}p{1e-5, 0.7}");

	// Print the TorchScript code
	std::cout<<"TorchScript:\n"<<model.getTorchScript()<<'\n';

	// Compile the TorchScript
	std::shared_ptr<torch::CompilationUnit> compiledModel = compileModel(model);

	// Create a range of omega values
	torch::Tensor omegas = torch::logspace(1, 4, 10, 10, torch::TensorOptions().device(device));
	std::cout<<"Omegas:\n"<<omegas<<"\n\n";

	// Run the 100th step in the parameter sweep of this model
	torch::Tensor spectra = runScriptModel(model, 0, compiledModel, omegas, device).cpu();

	std::cout<<"Spectra:\n"<<torch::view_as_real(spectra)<<'\n';
}
