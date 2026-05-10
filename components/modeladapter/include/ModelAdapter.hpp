#ifndef MODELADAPTER_HPP
#define MODELADAPTER_HPP

#include "simulation/contracts/ModelContract.hpp"
#include <chrono>

namespace emulator::model {

class Impl;

class ModelAdapter {
public:
    virtual ~ModelAdapter() = default;
    
    /// @name Жизненный цикл
    /// @{
    virtual bool initialize() = 0;
    virtual bool reset() = 0;
    /// @}
    
    /// @name Управление
    /// @{
    virtual bool setInputs(const ModelInputs& inputs) = 0;
    virtual bool step(
        std::chrono::milliseconds modelTime,
        std::chrono::milliseconds dt) = 0;
    /// @}
    
    /// @name Данные
    /// @{
    [[nodiscard]] virtual ModelOutputs readOutputs() const = 0;
    [[nodiscard]] virtual diagnostics::ModelDiagnosticsSnapshot diagnostics() const = 0;
    //Лучше через контрактный код (пример enum)
    //[[nodiscard]] virtual ModelStateCode state() const = 0;
    [[nodiscard]] virtual bool isRunning() const = 0;
    [[nodiscard]] virtual bool isEmergency() const = 0;
    /// @}
};

class ModelBase : public ModelAdapter {
public:
    ModelBase();
    ~ModelBase() override;

    bool initialize() override;
    bool reset() override;

    virtual bool setInputs(const ModelInputs& inputs) override;
    virtual bool step(
        std::chrono::milliseconds modelTime,
        std::chrono::milliseconds dt) override;

    [[nodiscard]] virtual ModelOutputs readOutputs() const override;
    [[nodiscard]] virtual diagnostics::ModelDiagnosticsSnapshot diagnostics() const override;
    [[nodiscard]] bool isRunning() const override;
    [[nodiscard]] bool isEmergency() const override;

private:
    std::unique_ptr<Impl> pimpl;
};

} 

#endif
