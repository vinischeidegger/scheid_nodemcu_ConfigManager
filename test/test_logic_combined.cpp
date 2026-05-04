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
    void SetUp() override {}
    void TearDown() override {}
};

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

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ConfigManagerTest, CanCreateWithDefaultOptions) {
    ConfigManager configManager;
    auto params = configManager.getParameters();
    EXPECT_EQ(params.size(), 0);
}

TEST_F(ConfigManagerTest, CanCreateWithCustomOptions) {
    ConfigManagerOptions options;
    options.pageTitle = "Custom Title";
    options.apSSID = "CustomAP";

    ConfigManager configManager(options);
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
#endif
