#pragma once

#include "SignalStateAspectBot.h"

#include "neokon-shared/signalling/UkSemaphoreAspectState.h"

#include "neokon-shared/signalling/BooleanAspectState.h"
#include "neokon-shared/signalling/LampMode.h"
#include "neokon-shared/signalling/SignalState.h"

#include <string>

class SignalStateUkSemaphore : public SignalState, private SignalStateAspectBot
{
public:
    struct PartialState
    {
        PartialState() = default;

        explicit PartialState(std::string const& serialized);

        /**
         * Methods to convert proto data
         * to Signal state fields for deserialization
         */
        std::optional<UkSemaphoreAspectState> fromIntToUkSemaphoreAspectState(int state) const;
        void fromProtoToUkSemaphoreAspectState(
            int protoVal, std::optional<UkSemaphoreAspectState>& target) const;
        void fromProtoToUkSemaphoreAspectState(
            int protoVal, std::optional<std::optional<UkSemaphoreAspectState>>& target) const;
        void fromProtoToUkSemaphoreAspectState(
            std::string const& protoVal, std::optional<std::string>& target) const;

        /**
         * Methods to convert Signal state fields
         * to proto data for serialization
         */
        int toUkSemaphoreAspectStateProto(UkSemaphoreAspectState value) const;
        int toUkSemaphoreAspectStateProto(
            std::optional<UkSemaphoreAspectState> const& value) const;
        int toUkSemaphoreAspectStateProto(
            std::optional<std::optional<UkSemaphoreAspectState>> const& value) const;
        std::string toUkSemaphoreAspectStateProto(std::optional<std::string> const& value) const;

        std::string serialize() const;

        /**
         * Why haven't we wrapped mainAspectState into an extra std::optional?
         * Because this field being optional in the outer class is something we
         * had to do rather than something we wanted to do. To minimise the extra
         * complexity that brings, we haven't wrapped it into an extra std::optional
         * here. After all, we don't want a partial state update being able to set
         * the main aspect state to null.
         *
         * All other fields we do wrap into an extra std::optional.
         *
         * @see SignalStateRuMetro::m_mainAspectState
         */
        std::optional<UkSemaphoreAspectState> m_mainAspectState;
        std::optional<std::optional<UkSemaphoreAspectState>> m_manuallyRequestedMainAspectState;
        std::optional<std::string> m_arm;
    };

    /**
     * Returns the partial state containing all the data from the full state.
     */
    PartialState toPartialState() const;

    virtual std::unique_ptr<SignalState> clone() const override;

    virtual CommonCabinSignalColor commonCabinSignalColor() const override;

    virtual bool mergeSerializedPartialState(std::string const& serialized) override;

    /**
     * Takes the fields from the partial state that are set, and writes
     * them to the corresponding fields in this object.
     *
     * Returns true if any fields of this object changed their values.
     */
    bool mergePartialState(PartialState const& partialState);

    const std::string& getArmUid() const { return m_arm; }

    std::optional<UkSemaphoreAspectState> mainAspectState() const { return m_mainAspectState; }

    std::optional<UkSemaphoreAspectState>
        manuallyRequestedMainAspectState() const { return m_manuallyRequestedMainAspectState; }

    /**
     * Returns mainAspectState() if it's set, or manuallyRequestedMainAspectState() otherwise.
     */
    std::optional<UkSemaphoreAspectState> actualOrRequestedMainAspectState() const;

private:
    std::optional<UkSemaphoreAspectState> m_mainAspectState;
    std::optional<UkSemaphoreAspectState> m_manuallyRequestedMainAspectState;
    std::string m_arm;
};
