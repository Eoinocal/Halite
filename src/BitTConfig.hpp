
struct BitTConfig
{
	BitTConfig() :
		maxConnections(50),
		maxUploads(50)
	{ }
	
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & maxConnections;
		ar & maxUploads;
	}

	int maxConnections;
	int maxUploads;
};