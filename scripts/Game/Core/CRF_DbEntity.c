[EDF_DbName("CRF_DbEntity")]
class CRF_DbEntity : EDF_DbEntity
{
    int m_iValue;

    //------------------------------------------------------------------------------------------------
    static CRF_DbEntity Create(int value)
    {
        CRF_DbEntity instance();
        instance.m_iValue = value;
        return instance;
    }
};