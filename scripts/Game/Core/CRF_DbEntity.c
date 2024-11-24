[EDF_DbName("CRF_DbEntity")]
class CRF_DbEntity : EDF_DbEntity
{
	string m_sName;
    int m_iValue;

    //------------------------------------------------------------------------------------------------
    static CRF_DbEntity CreatePlayer(string name, int value)
    {
        CRF_DbEntity instance();
		instance.m_sName = name;
        instance.m_iValue = value;
        return instance;
    }
};