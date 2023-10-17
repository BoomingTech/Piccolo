#pragma once

#include <string>
#include <vector>
#include "array.h"
#include <runtime/function/animation/lganim/common.h>


namespace Piccolo
{
	struct SNNLayer
	{
        Array2D<float> m_kernel;
        Array1D<float> m_bias;

		SNNLayer(int input_size, int neural_num)
		{
            m_kernel.resize(input_size, neural_num);
            m_bias.resize(neural_num);
		}
        SNNLayer() = default;
	};


	struct CNeuralNetwork
	{
        std::vector<SNNLayer> m_layers;
        Array1D<float> m_input_mean;
        Array1D<float> m_input_std;
        Array1D<float> m_output_mean;
        Array1D<float> m_output_std;



		void EvaluateNn(Array1D<float>& network_input, Array1D<float>& network_output)
		{
			// first normalize input
			// note: input is un-normalized feature
            Array1D<float> next_layer_input(network_input.size);
            int input_size = network_input.size;
            for (int i = 0; i < input_size; ++i)
			{
                next_layer_input(i) = (network_input(i) - m_input_mean(i)) / m_input_std(i);
			}


            // predict!
            for (uint32_t l = 0; l < m_layers.size(); l++)
            {
                const auto& layer = m_layers[l];
                Array1D<float> layer_output = layer.m_bias;
                for (int j = 0; j < layer.m_bias.size; ++j)
                {
                    for (int i = 0; i < next_layer_input.size; ++i)
	                {
                        layer_output(j) += next_layer_input(i) * layer.m_kernel(i, j);
	                }

					//do relu activate function for no final layer
					if (l != m_layers.size() - 1u)
					{
                        layer_output(j) = maxf(layer_output(j), 0.0f);
					}

                }
                next_layer_input = layer_output;
			}


			//denormalize
            network_output.resize(next_layer_input.size);
            input_size = next_layer_input.size;
            for (int i = 0; i < input_size; ++i)
            {
                network_output(i) = next_layer_input(i) * m_output_std(i) + m_output_mean(i);
            }
		}
	};



    inline void LoadNetwork(CNeuralNetwork& nn, const std::string& file_name)
    {
        FILE*   f;
        errno_t r = fopen_s(&f, file_name.c_str(), "rb");
        assert(f != NULL);

        array1d_read(nn.m_input_mean, f);
        array1d_read(nn.m_input_std, f);
        array1d_read(nn.m_output_mean, f);
        array1d_read(nn.m_output_std, f);

        int layer_count;
        size_t num = fread(&layer_count, sizeof(int), 1, f);

        nn.m_layers.resize(layer_count);
        for (int i = 0; i < layer_count; ++i)
        {
            array2d_read(nn.m_layers[i].m_kernel, f);
            array1d_read(nn.m_layers[i].m_bias, f);
        }

        r = fclose(f);
    }

    inline void LoadLatent(Array2D<float>& latent, const std::string& file_name)
    {
        FILE*   f;
        errno_t r = fopen_s(&f, file_name.c_str(), "rb");
        assert(f != NULL);

        array2d_read(latent, f);


        r = fclose(f);
    }
}
