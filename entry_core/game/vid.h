#pragma once

class VID 
{
	public:
		VID() : m_id(0), m_uuid() {}

		// Konstruktor z ID i UUID
		VID(DWORD id, const boost::uuids::uuid& uuid) : m_id(id), m_uuid(uuid) {}
		VID(const VID &rvid) : m_id(rvid.m_id), m_uuid(rvid.m_uuid) {}

		const VID & operator = (const VID & rhs) 
		{
			m_id = rhs.m_id;
			m_uuid = rhs.m_uuid;
			return *this;
		}

		bool operator == (const VID & rhs) const 
		{
			return (m_id == rhs.m_id) && (m_uuid == rhs.m_uuid);
		}

		bool operator != (const VID & rhs) const 
		{
			return !(*this == rhs);
		}

		operator DWORD() const 
		{
			return m_id;
		}

		void Reset() 
		{
			m_id = 0;
			m_uuid = boost::uuids::uuid(); // pusty UUID
		}

	private:
		DWORD m_id;
		boost::uuids::uuid m_uuid; //uuid
};
