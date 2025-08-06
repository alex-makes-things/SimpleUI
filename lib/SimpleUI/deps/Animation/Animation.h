#pragma once
#include <Arduino.h>
#include <stdint.h>

namespace SimpleUI{
    
    //Used to represent the completition state of an animation
    enum class AnimState{Start, Running, Finished};

    
    class Animation{
        public:
        float factor = 1.0f;
        bool loop = false;
        friend class AnimatedApp;

        public:
        /*!
            @param start Initial value
            @param end Final value
            @param length The duration of the animation in milliseconds
            @param step How much smoothing to apply (>1)
        */
        Animation(float start = 0.0f, float end = 1.0f, unsigned int length = 1000U, float step = 1.0f) 
        : m_start(start), m_end(end), m_length(length*1000U), m_now(micros()), m_startTime(micros()), m_progress(start), factor(step){}


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


        bool operator==(const AnimState state){
            return getState() == state;
        }

        bool operator!=(const AnimState state){
            return getState() != state;
        }

        static float smoothStep(const float x, const float k);

        static inline float lerp(const float a, const float b, const float f) { return (a * (1.0 - f)) + (b * f); }

        static inline float normalize(const float x, const float min, const float max){ return (x - min) / (max - min); }

        static inline float map(const float x, const float in_min, const float in_max, const float out_min, const float out_max){ return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; }
    
        static inline float clamp(const float x, const float y, const float z){
            if(y < z)
                return (x < y) ? y : (x > z) ? z : x; 
            else if (y > z)
                return (x < z) ? z : (x > y) ? y : x; 
            else
                return y;
        }

        private:
        static constexpr float EPSILON = 0.0005;
        
        AnimState m_state = AnimState::Start;
        float m_T = 0.0f, m_start, m_end, m_progress;
        unsigned int m_length;
        uint32_t m_elapsed = 0UL, m_now, m_startTime;
        bool m_enable = false;
    };

}
