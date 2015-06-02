//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <deque>

#include "catch.hpp"

#include "dll/dbn.hpp"

#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

TEST_CASE( "dbn/mnist_1", "dbn::simple" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<25>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(500);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10, 50);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);
}

TEST_CASE( "dbn/mnist_2", "dbn::containers" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<25>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::deque, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(200);
    dataset.training_labels.resize(200);

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 5);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 5, 50);

    REQUIRE(error < 5e-2);
}

TEST_CASE( "dbn/mnist_3", "dbn::labels" ) {
    auto dataset = mnist::read_dataset<std::vector, std::vector, double>();

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(1000);
    dataset.training_labels.resize(1000);

    mnist::binarize_dataset(dataset);

    typedef dll::dbn_desc<
        dll::dbn_label_layers<
        dll::rbm_desc<28 * 28, 200, dll::batch_size<50>, dll::init_weights, dll::momentum>::rbm_t,
        dll::rbm_desc<200, 300, dll::batch_size<50>, dll::momentum>::rbm_t,
        dll::rbm_desc<310, 500, dll::batch_size<50>, dll::momentum>::rbm_t>>::dbn_t dbn_simple_t;

    auto dbn = std::make_unique<dbn_simple_t>();

    dbn->train_with_labels(dataset.training_images, dataset.training_labels, 10, 10);

    auto error = dll::test_set(dbn, dataset.training_images, dataset.training_labels, dll::label_predictor());
    std::cout << "test_error:" << error << std::endl;
    REQUIRE(error < 0.3);
}

TEST_CASE( "dbn/mnist_6", "dbn::cg_gaussian" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 200, dll::momentum, dll::batch_size<25>, dll::visible<dll::unit_type::GAUSSIAN>>::rbm_t,
        dll::rbm_desc<200, 500, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<500, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>
    >::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::deque, double>(1000);

    REQUIRE(!dataset.training_images.empty());

    mnist::normalize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10, 50);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);
}

//This test should not perform well, but should not fail
TEST_CASE( "dbn/mnist_8", "dbn::cg_relu" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::RELU>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::deque, double>(200);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10, 50);

    REQUIRE(std::isfinite(error));

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;
}

TEST_CASE( "dbn/mnist_15", "dbn::parallel" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::parallel, dll::batch_size<25>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::parallel, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::parallel, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(500);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10, 50);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);
}

TEST_CASE( "dbn/mnist_16", "dbn::fast" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<5>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<5>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<5>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(25);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 5);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 2, 5);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);
}

//{{{ Performance debugging tests

TEST_CASE( "dbn/mnist_101", "dbn::slow_parallel" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 300, dll::momentum, dll::parallel, dll::batch_size<24>, dll::init_weights>::rbm_t,
        dll::rbm_desc<300, 1000, dll::momentum, dll::parallel, dll::batch_size<24>>::rbm_t,
        dll::rbm_desc<1000, 10, dll::momentum, dll::parallel, dll::batch_size<24>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(2000);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
}

TEST_CASE( "dbn/mnist_102", "[dbn][bench][slow]" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 300, dll::momentum, dll::batch_size<24>, dll::init_weights>::rbm_t,
        dll::rbm_desc<300, 1000, dll::momentum, dll::batch_size<24>>::rbm_t,
        dll::rbm_desc<1000, 10, dll::momentum, dll::batch_size<24>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(2000);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
}

//}}}

TEST_CASE( "dbn/mnist_17", "dbn::memory" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<25>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>
    , dll::memory>::dbn_t dbn_t;

    auto dataset = mnist::read_dataset<std::vector, std::vector, double>(500);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10, 50);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);
}
