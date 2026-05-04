#ifndef ARDUINO
#include "mock_arduino.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Include the classes to test
#include "ConfigData.h"
#include "ConfigManager.h"

// Test fixture for ConfigData
class ConfigDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

// Basic test for ConfigData
TEST_F(ConfigDataTest, CanRegisterParameter) {
    ConfigData configData;

    int testValue = 42;
    configData.registerParameter("test_param", &testValue, ParamType::INT, "Test Parameter");

    auto params = configData.getParameters();
    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0].id.c_str(), std::string("test_param"));
    EXPECT_EQ(params[0].type, ParamType::INT);
    EXPECT_EQ(params[0].label.c_str(), std::string("Test Parameter"));
}

TEST_F(ConfigDataTest, CanGetParameters) {
    ConfigData configData;

    float testValue = 3.14f;
    configData.registerParameter("float_param", &testValue, ParamType::FLOAT, "Float Parameter");

    auto params = configData.getParameters();
    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(*static_cast<float*>(params[0].valuePointer), 3.14f);
}

// Test fixture for ConfigManager
class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup before each test
    }

    void TearDown() override {
        // Cleanup after each test
    }
};

// Basic test for ConfigManager
TEST_F(ConfigManagerTest, CanCreateWithDefaultOptions) {
    ConfigManager configManager;

    // Test that we can get parameters (should be empty initially)
    auto params = configManager.getParameters();
    EXPECT_EQ(params.size(), 0);
}

TEST_F(ConfigManagerTest, CanCreateWithCustomOptions) {
    ConfigManagerOptions options;
    options.pageTitle = "Custom Title";
    options.apSSID = "CustomAP";

    ConfigManager configManager(options);

    // Test that page title is set
    EXPECT_EQ(configManager.getPageTitle().c_str(), std::string("Custom Title"));
}

TEST_F(ConfigManagerTest, CanRegisterParameter) {
    ConfigManager configManager;

    int testValue = 123;
    configManager.registerParameter("test_int", &testValue, ParamType::INT, "Test Integer");

    auto params = configManager.getParameters();
    ASSERT_EQ(params.size(), 1);
    EXPECT_EQ(params[0].id.c_str(), std::string("test_int"));
    EXPECT_EQ(params[0].type, ParamType::INT);
}
#else
// Unity tests for embedded platform
#include <unity.h>
#include "ConfigData.h"
#include "ConfigManager.h"

void test_configdata_can_register_parameter(void) {
    ConfigData configData;

    int testValue = 42;
    configData.registerParameter("test_param", &testValue, ParamType::INT, "Test Parameter");

    auto params = configData.getParameters();
    TEST_ASSERT_EQUAL(1, params.size());
    TEST_ASSERT_EQUAL_STRING("test_param", params[0].id.c_str());
    TEST_ASSERT_EQUAL(ParamType::INT, params[0].type);
    TEST_ASSERT_EQUAL_STRING("Test Parameter", params[0].label.c_str());
}

void test_configdata_can_get_parameters(void) {
    ConfigData configData;

    float testValue = 3.14f;
    configData.registerParameter("float_param", &testValue, ParamType::FLOAT, "Float Parameter");

    auto params = configData.getParameters();
    TEST_ASSERT_EQUAL(1, params.size());
    TEST_ASSERT_EQUAL(3.14f, *static_cast<float*>(params[0].valuePointer));
}

void test_configmanager_can_create_with_default_options(void) {
    ConfigManager configManager;

    // Test that we can get parameters (should be empty initially)
    auto params = configManager.getParameters();
    TEST_ASSERT_EQUAL(0, params.size());
}

void test_configmanager_can_create_with_custom_options(void) {
    ConfigManagerOptions options;
    options.pageTitle = "Custom Title";
    options.apSSID = "CustomAP";

    ConfigManager configManager(options);

    // Test that page title is set
    TEST_ASSERT_EQUAL_STRING("Custom Title", configManager.getPageTitle().c_str());
}

void test_configmanager_can_register_parameter(void) {
    ConfigManager configManager;

    int testValue = 123;
    configManager.registerParameter("test_int", &testValue, ParamType::INT, "Test Integer");

    auto params = configManager.getParameters();
    TEST_ASSERT_EQUAL(1, params.size());
    TEST_ASSERT_EQUAL_STRING("test_int", params[0].id.c_str());
    TEST_ASSERT_EQUAL(ParamType::INT, params[0].type);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_configdata_can_register_parameter);
    RUN_TEST(test_configdata_can_get_parameters);
    RUN_TEST(test_configmanager_can_create_with_default_options);
    RUN_TEST(test_configmanager_can_create_with_custom_options);
    RUN_TEST(test_configmanager_can_register_parameter);
    UNITY_END();
}

void loop() {
    // Nothing to do here for tests
}
#endif