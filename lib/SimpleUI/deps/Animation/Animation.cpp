#include "Animation.h"
#include <math.h>

namespace SimpleUI{

    float Animation::smoothStep(const float x, const float k){
        float xk = std::pow(x, k);
        return xk / (xk + std::pow(1-x, k));
    }

    void Animation::Start(){
        m_enable = true;
        m_startTime = micros();
    }

    void Animation::Resume(){
        m_enable = true; 
        m_startTime = micros() - m_elapsed;
    };

    void Animation::Reset(){
        m_startTime = micros();
        m_elapsed = 0UL;
        m_progress = m_start;
        m_state = AnimState::Start;
    }
    
    void Animation::Flip(){
        float temp = m_start;
        m_start = m_end;
        m_end = temp;
        m_elapsed = m_length - m_elapsed;
        m_startTime = m_now - m_elapsed;
    }

    const AnimState Animation::getState() const {
        if (fabs(m_progress - m_end) <= EPSILON || m_state == AnimState::Finished)
            return AnimState::Finished;
        else if (fabs(m_progress - m_start) <= EPSILON || m_state == AnimState::Start)
            return AnimState::Start;
        else
            return AnimState::Running;
    }


    void Animation::Update(){
        
        if (m_enable) {
            if ( m_state != AnimState::Finished) 
            {
                m_now = micros();
                m_elapsed = m_now - m_startTime;

                m_T = normalize(static_cast<float>(m_elapsed), 0, m_length);
                m_T = std::clamp(m_T, 0.0f, 1.0f);

                /*
                if(func == Interpolation::Sinusoidal) 
                    m_T = static_cast<float>(0.5*sin(m_T*M_PI-M_PI_2)+0.5);
                else if (func == Interpolation::SmoothStep)
                    m_T = smoothStep(m_T, smoothstep);
                */
                m_T = smoothStep(m_T, factor);

                m_progress = Animation::clamp(lerp(m_start, m_end, m_T), m_start, m_end);


                m_state = m_elapsed >= m_length ? AnimState::Finished : AnimState::Running;
            }
            else{
                if (loop)
                    Reset();
            }
        }
    }

}
