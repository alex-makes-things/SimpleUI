#pragma once
#include <Arduino.h>
#include <stdint.h>

namespace SimpleUI{
    
    //Used to represent the completition state of an animation
    enum class AnimState{Start, Running, Finished};
    //Interpolation algorithm
    enum class Interpolation{Linear, CubicBezier, Sinusoidal};
    
    class Animation{
        public:
        /*!
            @param start Initial value
            @param end Final value
            @param length The duration of the animation in milliseconds
        */
        Animation(float start = 0.0f, float end = 1.0f, unsigned int length = 1000U, Interpolation interpolation = Interpolation::Linear) 
        : m_start(start), m_end(end), m_length(length*1000U),
          m_now(micros()), m_startTime(micros()), m_elapsed(0UL), m_progress(start),
          m_enable(false), m_loop(false), m_state(AnimState::Start), m_T(0.0f)
          { setFunc(interpolation); }

        void Start();
        //To be called after a pause
        void Resume();
        inline void Pause() { m_enable = false; };
        //Restart the animation
        void Reset();
        void Flip();
        void Update();

        /*!
            @brief Describes the completition state of the animation
            @param AnimState::Start
            @param AnimState::Running
            @param AnimState::Finished
        */
        const AnimState getState() const;

        /// @return True if the animation goes forwards
        inline const bool getDirection() const { return m_start < m_end; };
        /// @return The interpolated value
        inline const float getProgress() const { return m_progress; }
        inline const bool isEnabled() const { return m_enable; }
        inline void setLoop(const bool loop){ m_loop = loop; }
        void setFunc(Interpolation function);

        bool operator==(const AnimState state){
            return getState() == state;
        }

        static inline float lerp(const float a, const float b, const float f) { return (a * (1.0 - f)) + (b * f); }

        static inline float normalize(const float x, const float min, const float max){ return (x - min) / (max - min); }

        static inline float map(const float x, const float in_min, const float in_max, const float out_min, const float out_max){ return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; }
    
        private:
        static constexpr float EPSILON = 0.00005;
        Interpolation m_interpolation;
        AnimState m_state;
        float m_start, m_end, m_progress, m_T;
        unsigned int m_length;
        uint32_t m_startTime, m_elapsed, m_now;
        bool m_enable, m_loop, m_flipped;
    };

}
