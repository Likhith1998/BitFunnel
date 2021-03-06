// The MIT License (MIT)

// Copyright (c) 2016, Microsoft

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "BitFunnel/Index/IFactSet.h"  // For FactHandle.
#include "BitFunnel/Index/ITermTable.h"
#include "BitFunnel/Index/RowIdSequence.h"


namespace BitFunnel
{
    //*************************************************************************
    //
    // RowIdSequence
    //
    //*************************************************************************
    // TODO: should GetRawHash be GetClassifiedHash or GetGeneralHash?
    RowIdSequence::RowIdSequence(Term const & term, ITermTable const & termTable)
      : m_hash(term.GetRawHash()),
        m_termTable(termTable)
    {
        // TODO: Get rid of out parameter for m_termKind. Consider returning an std::pair.
        // TODO: See if there is any way this can run in the initializer so that members
        // can be const.
        // TODO: Eliminate Shard and Tier from RowId.
        const PackedRowIdSequence packed(termTable.GetRows(term));
        Initialize(packed);
    }


    //// TODO: Implement this constructor.
    RowIdSequence::RowIdSequence(FactHandle fact, ITermTable const & termTable)
    : m_hash(0),
      m_termTable(termTable)
    {
        // We're throwing because this isn't implemented yet.  This if is
        // because, on some configurations of VC++, throwing causes the build to
        // error out. For reasons we don't understand, this doesn't happen in
        // AppVeyor CI.
        if (fact != 17u)
        {
            throw fact;
        }

      // // TODO: Figure out how to eliminate StreamId::Metaword for facts.
      // // Do we just create c_MetaWordStreamId? Can user's use this StreamId?
      // const Term term(static_cast<Term::Hash>(fact), // TODO: need to figure out if we need to subtract 3 (or whatever)
      //                 StreamId::MetaWord, // TODO: can pass dummy value since TermTable::GetRows doesn't use thsi for facts.
      //                 static_cast<Term::IdfX10>(0)); // TODO: can pass dummy vaue for fact.

      // const PackedTermInfo info = termTable.GetTermInfo(term, m_termKind);
      // Initialize(info);
    }


    RowIdSequence::const_iterator RowIdSequence::begin() const
    {
        return const_iterator(*this, 0ull);
    }


    RowIdSequence::const_iterator RowIdSequence::end() const
    {
        return const_iterator(*this, m_rowIdCount);
    }


    RowId RowIdSequence::GetRow(size_t row) const
    {
        if (row >= m_rowIdCount)
        {
            RecoverableError error("RowIdSequence::GetRow(): row out of range.");
            throw error;
        }

        if (m_type == PackedRowIdSequence::Type::Explicit)
        {
            return m_termTable.GetRowIdExplicit(m_rowIdStart + row);
        }
        else if(m_type == PackedRowIdSequence::Type::Adhoc)
        {
            // TODO: can hash variant be created here in order to eliminate
            // third argument.
            return m_termTable.GetRowIdAdhoc(m_hash,
                                             m_rowIdStart + row,
                                             row);
        }
        else
        {
            return m_termTable.GetRowIdFact(m_rowIdStart + row);
        }

        // No special clause for ITermTable::Disposed as they return an empty
        // PackedTermInfo which does not support enumeration.
    }


    void RowIdSequence::Initialize(PackedRowIdSequence const & packed)
    {
        m_rowIdStart = packed.GetStart();
        m_rowIdCount = packed.GetEnd() - packed.GetStart();
        m_type = packed.GetType();
    }
}
