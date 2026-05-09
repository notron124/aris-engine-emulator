#ifndef MODELDIAGNOSTICS_HPP
#define MODELDIAGNOSTICS_HPP

#include <QString>

namespace emulator::model::diagnostics {

/**
 * @brief Код диагностической ошибки расчётной модели.
 *
 * Используется только на границе взаимодействия с моделью. Слой simulation
 * преобразует эти ошибки в собственную SimulationDiagnosticsSnapshot.
 */
enum class ModelFaultCode : int {
    /**
     * @brief Ошибки нет.
     */
    None = 0,

    /**
     * @brief Модель перешла в аварийный режим по собственной логике.
     */
    Emergency = 1,

    /**
     * @brief Модель не может продолжать нормальный расчёт.
     */
    ModelFault = 2
};

/**
 * @brief Диагностический снимок расчётной модели.
 *
 * Этот тип принадлежит контракту взаимодействия с моделью. Он не должен
 * напрямую публиковаться наружу как выходной DTO слоя simulation.
 */
struct ModelDiagnosticsSnapshot {
    ModelFaultCode faultCode = ModelFaultCode::None;
    /// Текстовое описание ошибки модели для логов и диагностики.
    QString message;

    [[nodiscard]] bool hasFault() const noexcept
    {
        return faultCode != ModelFaultCode::None;
    }
    [[nodiscard]] bool hasFault(ModelFaultCode code) const noexcept
    {
        return faultCode == code;
    }
};

} // namespace emulator::model::diagnostics

#endif // MODELDIAGNOSTICS_HPP
