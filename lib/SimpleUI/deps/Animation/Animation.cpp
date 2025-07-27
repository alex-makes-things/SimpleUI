#include "Animation.h"
#include <math.h>

namespace SimpleUI{

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

    void Animation::setFunc(Interpolation function){
        assert(function != Interpolation::CubicBezier && "Cubic-Bezier has not been implemented yet.");
        m_interpolation = function;
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

                if(m_interpolation == Interpolation::Sinusoidal) 
                    m_T = static_cast<float>(0.5*sin(m_T*M_PI-M_PI_2)+0.5);
                
                m_T = std::clamp(m_T, 0.0f, 1.0f);

                m_progress = lerp(m_start, m_end, m_T);


                m_state = m_elapsed >= m_length ? AnimState::Finished : AnimState::Running;
            }
            else{
                if (m_loop)
                    Reset();
            }
        }
    }

}
