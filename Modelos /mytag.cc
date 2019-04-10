/* ========================================================================
 * MyTag class
 *
 * Inherited from Tag class
 *
 * Create and add tags to packets
 *
 * ========================================================================
 */

class MyTag : public Tag
{
public:

  static TypeId 		GetTypeId (void);
  virtual TypeId 		GetInstanceTypeId (void) const;
  virtual uint32_t 		GetSerializedSize (void) const;
  virtual void 			Serialize (TagBuffer i) const;
  virtual void 			Deserialize (TagBuffer i);
  virtual void 			Print (std::ostream &os) const;
  void 					SetSimpleValue (uint8_t value);
  uint8_t 				GetSimpleValue (void) const;

private:
  uint8_t 				m_simpleValue;  //!< tag value
};


/**
 * \brief Get the type ID.
 * \return the object TypeId
 */

TypeId
MyTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MyTag")
    .SetParent<Tag> ()
    .AddConstructor<MyTag> ()
    .AddAttribute ("SimpleValue",
                   "A simple value",
                   EmptyAttributeValue (),
                   MakeUintegerAccessor (&MyTag::GetSimpleValue),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId
MyTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MyTag::GetSerializedSize (void) const
{
  return 1;
}

void
MyTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_simpleValue);
}

void
MyTag::Deserialize (TagBuffer i)
{
  m_simpleValue = i.ReadU8 ();
}

void
MyTag::Print (std::ostream &os) const
{
  //os << "v=" << (uint32_t)m_simpleValue;
  std::cout << "Tag " << (uint32_t)m_simpleValue << std::endl;
}


/**
 * Set the tag value
 * \param value The tag value.
 */
void
MyTag::SetSimpleValue (uint8_t value)
{
  m_simpleValue = value;
}


/**
 * Get the tag value
 * \return the tag value.
 */

uint8_t
MyTag::GetSimpleValue (void) const
{
  return m_simpleValue;
}

/* ------------------------------------------------------------------------
 * End of MyTag class
 * ------------------------------------------------------------------------
 */
