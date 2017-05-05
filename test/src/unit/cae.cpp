//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <deque>

#include "dll_test.hpp"

#include "dll/neural/conv_layer.hpp"
#include "dll/neural/deconv_layer.hpp"
#include "dll/pooling/mp_layer.hpp"
#include "dll/pooling/upsample_layer.hpp"
#include "dll/dbn.hpp"

#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

TEST_CASE("conv/ae/1", "[dense][dbn][mnist][sgd][ae]") {
    typedef dll::dbn_desc<
        dll::dbn_layers<
            dll::conv_desc<1, 28, 28, 2, 5, 5, dll::activation<dll::function::SIGMOID>>::layer_t,
            dll::mp_layer_3d_desc<2, 24, 24, 1, 2, 2>::layer_t,
            dll::upsample_layer_3d_desc<2, 12, 12, 1, 2, 2>::layer_t,
            dll::deconv_desc<2, 24, 24, 1, 5, 5, dll::activation<dll::function::SIGMOID>>::layer_t
        >, dll::trainer<dll::sgd_trainer>, dll::batch_size<20>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::fast_dyn_matrix<float, 1, 28, 28>>(1000);
    REQUIRE(!dataset.training_images.empty());

    dll_test::mnist_scale(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->display();

    dbn->learning_rate = 0.1;

    auto ft_error = dbn->fine_tune_ae(dataset.training_images, 25);
    std::cout << "ft_error:" << ft_error << std::endl;

    CHECK(ft_error < 0.3);

    auto test_error = dll::test_set_ae(*dbn, dataset.test_images);
    std::cout << "test_error:" << test_error << std::endl;
    REQUIRE(test_error < 0.3);
}
