#ifndef HOPMOD_FREQLIMIT_HPP
#define HOPMOD_FREQLIMIT_HPP

class freqlimit
{
public:
    freqlimit(int length)
     :m_length(length),
      m_hit(0)
    {
        
    }
    
    int next(int time)
    {
        if(time >= m_hit)
        {
            m_hit = time + m_length;
            return 0;
        }else return m_hit - time; 
    }
private:
    int m_length;
    int m_hit;
};

#endif
