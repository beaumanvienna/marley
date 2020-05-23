//
// Copyright (C) 2014-2016 LunarG, Inc.
// Copyright (C) 2015-2020 Google, Inc.
// Copyright (C) 2017 ARM Limited.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

//
// Visit the nodes in the glslang intermediate tree representation to
// translate them to SPIR-V.
//

#include "spirv.hpp"
#include "GlslangToSpv.h"
#include "SpvBuilder.h"
namespace Pspv {
    #include "GLSL.std.450.h"
    #include "GLSL.ext.KHR.h"
    #include "GLSL.ext.EXT.h"
    #include "GLSL.ext.AMD.h"
    #include "GLSL.ext.NV.h"
}

// Glslang includes
#include "../glslang/MachineIndependent/localintermediate.h"
#include "../glslang/MachineIndependent/SymbolTable.h"
#include "../glslang/Include/Common.h"
#include "../glslang/Include/revision.h"

#include <fstream>
#include <iomanip>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace {

namespace {
class SpecConstantOpModeGuard {
public:
    SpecConstantOpModeGuard(Pspv::Builder* builder)
        : builder_(builder) {
        previous_flag_ = builder->isInSpecConstCodeGenMode();
    }
    ~SpecConstantOpModeGuard() {
        previous_flag_ ? builder_->setToSpecConstCodeGenMode()
                       : builder_->setToNormalCodeGenMode();
    }
    void turnOnSpecConstantOpMode() {
        builder_->setToSpecConstCodeGenMode();
    }

private:
    Pspv::Builder* builder_;
    bool previous_flag_;
};

struct OpDecorations {
    public:
        OpDecorations(Pspv::Decoration precision, Pspv::Decoration noContraction, Pspv::Decoration nonUniform) :
            precision(precision)
#ifndef GLSLANG_WEB
            ,
            noContraction(noContraction),
            nonUniform(nonUniform)
#endif
        { }

    Pspv::Decoration precision;

#ifdef GLSLANG_WEB
        void addNoContraction(Pspv::Builder&, Pspv::Id) const { }
        void addNonUniform(Pspv::Builder&, Pspv::Id) const { }
#else
        void addNoContraction(Pspv::Builder& builder, Pspv::Id t) { builder.addDecoration(t, noContraction); }
        void addNonUniform(Pspv::Builder& builder, Pspv::Id t)  { builder.addDecoration(t, nonUniform); }
    protected:
        Pspv::Decoration noContraction;
        Pspv::Decoration nonUniform;
#endif

};

} // namespace

//
// The main holder of information for translating glslang to SPIR-V.
//
// Derives from the AST walking base class.
//
class TGlslangToSpvTraverser : public Pglslang::TIntermTraverser {
public:
    TGlslangToSpvTraverser(unsigned int spvVersion, const Pglslang::TIntermediate*, Pspv::SpvBuildLogger* logger,
        Pglslang::SpvOptions& options);
    virtual ~TGlslangToSpvTraverser() { }

    bool visitAggregate(Pglslang::TVisit, Pglslang::TIntermAggregate*);
    bool visitBinary(Pglslang::TVisit, Pglslang::TIntermBinary*);
    void visitConstantUnion(Pglslang::TIntermConstantUnion*);
    bool visitSelection(Pglslang::TVisit, Pglslang::TIntermSelection*);
    bool visitSwitch(Pglslang::TVisit, Pglslang::TIntermSwitch*);
    void visitSymbol(Pglslang::TIntermSymbol* symbol);
    bool visitUnary(Pglslang::TVisit, Pglslang::TIntermUnary*);
    bool visitLoop(Pglslang::TVisit, Pglslang::TIntermLoop*);
    bool visitBranch(Pglslang::TVisit visit, Pglslang::TIntermBranch*);

    void finishSpv();
    void dumpSpv(std::vector<unsigned int>& out);

protected:
    TGlslangToSpvTraverser(TGlslangToSpvTraverser&);
    TGlslangToSpvTraverser& operator=(TGlslangToSpvTraverser&);

    Pspv::Decoration TranslateInterpolationDecoration(const Pglslang::TQualifier& qualifier);
    Pspv::Decoration TranslateAuxiliaryStorageDecoration(const Pglslang::TQualifier& qualifier);
    Pspv::Decoration TranslateNonUniformDecoration(const Pglslang::TQualifier& qualifier);
    Pspv::Builder::AccessChain::CoherentFlags TranslateCoherent(const Pglslang::TType& type);
    Pspv::MemoryAccessMask TranslateMemoryAccess(const Pspv::Builder::AccessChain::CoherentFlags &coherentFlags);
    Pspv::ImageOperandsMask TranslateImageOperands(const Pspv::Builder::AccessChain::CoherentFlags &coherentFlags);
    Pspv::Scope TranslateMemoryScope(const Pspv::Builder::AccessChain::CoherentFlags &coherentFlags);
    Pspv::BuiltIn TranslateBuiltInDecoration(Pglslang::TBuiltInVariable, bool memberDeclaration);
    Pspv::ImageFormat TranslateImageFormat(const Pglslang::TType& type);
    Pspv::SelectionControlMask TranslateSelectionControl(const Pglslang::TIntermSelection&) const;
    Pspv::SelectionControlMask TranslateSwitchControl(const Pglslang::TIntermSwitch&) const;
    Pspv::LoopControlMask TranslateLoopControl(const Pglslang::TIntermLoop&, std::vector<unsigned int>& operands) const;
    Pspv::StorageClass TranslateStorageClass(const Pglslang::TType&);
    void addIndirectionIndexCapabilities(const Pglslang::TType& baseType, const Pglslang::TType& indexType);
    Pspv::Id createSpvVariable(const Pglslang::TIntermSymbol*, Pspv::Id forcedType);
    Pspv::Id getSampledType(const Pglslang::TSampler&);
    Pspv::Id getInvertedSwizzleType(const Pglslang::TIntermTyped&);
    Pspv::Id createInvertedSwizzle(Pspv::Decoration precision, const Pglslang::TIntermTyped&, Pspv::Id parentResult);
    void convertSwizzle(const Pglslang::TIntermAggregate&, std::vector<unsigned>& swizzle);
    Pspv::Id convertGlslangToSpvType(const Pglslang::TType& type, bool forwardReferenceOnly = false);
    Pspv::Id convertGlslangToSpvType(const Pglslang::TType& type, Pglslang::TLayoutPacking, const Pglslang::TQualifier&,
        bool lastBufferBlockMember, bool forwardReferenceOnly = false);
    bool filterMember(const Pglslang::TType& member);
    Pspv::Id convertGlslangStructToSpvType(const Pglslang::TType&, const Pglslang::TTypeList* glslangStruct,
                                          Pglslang::TLayoutPacking, const Pglslang::TQualifier&);
    void decorateStructType(const Pglslang::TType&, const Pglslang::TTypeList* glslangStruct, Pglslang::TLayoutPacking,
                            const Pglslang::TQualifier&, Pspv::Id);
    Pspv::Id makeArraySizeId(const Pglslang::TArraySizes&, int dim);
    Pspv::Id accessChainLoad(const Pglslang::TType& type);
    void    accessChainStore(const Pglslang::TType& type, Pspv::Id rvalue);
    void multiTypeStore(const Pglslang::TType&, Pspv::Id rValue);
    Pglslang::TLayoutPacking getExplicitLayout(const Pglslang::TType& type) const;
    int getArrayStride(const Pglslang::TType& arrayType, Pglslang::TLayoutPacking, Pglslang::TLayoutMatrix);
    int getMatrixStride(const Pglslang::TType& matrixType, Pglslang::TLayoutPacking, Pglslang::TLayoutMatrix);
    void updateMemberOffset(const Pglslang::TType& structType, const Pglslang::TType& memberType, int& currentOffset,
                            int& nextOffset, Pglslang::TLayoutPacking, Pglslang::TLayoutMatrix);
    void declareUseOfStructMember(const Pglslang::TTypeList& members, int glslangMember);

    bool isShaderEntryPoint(const Pglslang::TIntermAggregate* node);
    bool writableParam(Pglslang::TStorageQualifier) const;
    bool originalParam(Pglslang::TStorageQualifier, const Pglslang::TType&, bool implicitThisParam);
    void makeFunctions(const Pglslang::TIntermSequence&);
    void makeGlobalInitializers(const Pglslang::TIntermSequence&);
    void visitFunctions(const Pglslang::TIntermSequence&);
    void handleFunctionEntry(const Pglslang::TIntermAggregate* node);
    void translateArguments(const Pglslang::TIntermAggregate& node, std::vector<Pspv::Id>& arguments, Pspv::Builder::AccessChain::CoherentFlags &lvalueCoherentFlags);
    void translateArguments(Pglslang::TIntermUnary& node, std::vector<Pspv::Id>& arguments);
    Pspv::Id createImageTextureFunctionCall(Pglslang::TIntermOperator* node);
    Pspv::Id handleUserFunctionCall(const Pglslang::TIntermAggregate*);

    Pspv::Id createBinaryOperation(Pglslang::TOperator op, OpDecorations&, Pspv::Id typeId, Pspv::Id left, Pspv::Id right,
                                  Pglslang::TBasicType typeProxy, bool reduceComparison = true);
    Pspv::Id createBinaryMatrixOperation(Pspv::Op, OpDecorations&, Pspv::Id typeId, Pspv::Id left, Pspv::Id right);
    Pspv::Id createUnaryOperation(Pglslang::TOperator op, OpDecorations&, Pspv::Id typeId, Pspv::Id operand,
                                 Pglslang::TBasicType typeProxy, const Pspv::Builder::AccessChain::CoherentFlags &lvalueCoherentFlags);
    Pspv::Id createUnaryMatrixOperation(Pspv::Op op, OpDecorations&, Pspv::Id typeId, Pspv::Id operand,
                                       Pglslang::TBasicType typeProxy);
    Pspv::Id createConversion(Pglslang::TOperator op, OpDecorations&, Pspv::Id destTypeId, Pspv::Id operand,
                             Pglslang::TBasicType typeProxy);
    Pspv::Id createIntWidthConversion(Pglslang::TOperator op, Pspv::Id operand, int vectorSize);
    Pspv::Id makeSmearedConstant(Pspv::Id constant, int vectorSize);
    Pspv::Id createAtomicOperation(Pglslang::TOperator op, Pspv::Decoration precision, Pspv::Id typeId, std::vector<Pspv::Id>& operands, Pglslang::TBasicType typeProxy, const Pspv::Builder::AccessChain::CoherentFlags &lvalueCoherentFlags);
    Pspv::Id createInvocationsOperation(Pglslang::TOperator op, Pspv::Id typeId, std::vector<Pspv::Id>& operands, Pglslang::TBasicType typeProxy);
    Pspv::Id CreateInvocationsVectorOperation(Pspv::Op op, Pspv::GroupOperation groupOperation, Pspv::Id typeId, std::vector<Pspv::Id>& operands);
    Pspv::Id createSubgroupOperation(Pglslang::TOperator op, Pspv::Id typeId, std::vector<Pspv::Id>& operands, Pglslang::TBasicType typeProxy);
    Pspv::Id createMiscOperation(Pglslang::TOperator op, Pspv::Decoration precision, Pspv::Id typeId, std::vector<Pspv::Id>& operands, Pglslang::TBasicType typeProxy);
    Pspv::Id createNoArgOperation(Pglslang::TOperator op, Pspv::Decoration precision, Pspv::Id typeId);
    Pspv::Id getSymbolId(const Pglslang::TIntermSymbol* node);
    void addMeshNVDecoration(Pspv::Id id, int member, const Pglslang::TQualifier & qualifier);
    Pspv::Id createSpvConstant(const Pglslang::TIntermTyped&);
    Pspv::Id createSpvConstantFromConstUnionArray(const Pglslang::TType& type, const Pglslang::TConstUnionArray&, int& nextConst, bool specConstant);
    bool isTrivialLeaf(const Pglslang::TIntermTyped* node);
    bool isTrivial(const Pglslang::TIntermTyped* node);
    Pspv::Id createShortCircuit(Pglslang::TOperator, Pglslang::TIntermTyped& left, Pglslang::TIntermTyped& right);
    Pspv::Id getExtBuiltins(const char* name);
    std::pair<Pspv::Id, Pspv::Id> getForcedType(Pspv::BuiltIn, const Pglslang::TType&);
    Pspv::Id translateForcedType(Pspv::Id object);
    Pspv::Id createCompositeConstruct(Pspv::Id typeId, std::vector<Pspv::Id> constituents);

    Pglslang::SpvOptions& options;
    Pspv::Function* shaderEntry;
    Pspv::Function* currentFunction;
    Pspv::Instruction* entryPoint;
    int sequenceDepth;

    Pspv::SpvBuildLogger* logger;

    // There is a 1:1 mapping between a spv builder and a module; this is thread safe
    Pspv::Builder builder;
    bool inEntryPoint;
    bool entryPointTerminated;
    bool linkageOnly;                  // true when visiting the set of objects in the AST present only for establishing interface, whether or not they were statically used
    std::set<Pspv::Id> iOSet;           // all input/output variables from either static use or declaration of interface
    const Pglslang::TIntermediate* glslangIntermediate;
    bool nanMinMaxClamp;               // true if use NMin/NMax/NClamp instead of FMin/FMax/FClamp
    Pspv::Id stdBuiltins;
    std::unordered_map<const char*, Pspv::Id> extBuiltinMap;

    std::unordered_map<int, Pspv::Id> symbolValues;
    std::unordered_set<int> rValueParameters;  // set of formal function parameters passed as rValues, rather than a pointer
    std::unordered_map<std::string, Pspv::Function*> functionMap;
    std::unordered_map<const Pglslang::TTypeList*, Pspv::Id> structMap[Pglslang::ElpCount][Pglslang::ElmCount];
    // for mapping glslang block indices to spv indices (e.g., due to hidden members):
    std::unordered_map<int, std::vector<int>> memberRemapper;
    // for mapping glslang symbol struct to symbol Id
    std::unordered_map<const Pglslang::TTypeList*, int> glslangTypeToIdMap;
    std::stack<bool> breakForLoop;  // false means break for switch
    std::unordered_map<std::string, const Pglslang::TIntermSymbol*> counterOriginator;
    // Map pointee types for EbtReference to their forward pointers
    std::map<const Pglslang::TType *, Pspv::Id> forwardPointers;
    // Type forcing, for when SPIR-V wants a different type than the AST,
    // requiring local translation to and from SPIR-V type on every access.
    // Maps <builtin-variable-id -> AST-required-type-id>
    std::unordered_map<Pspv::Id, Pspv::Id> forceType;
};

//
// Helper functions for translating glslang representations to SPIR-V enumerants.
//

// Translate glslang profile to SPIR-V source language.
Pspv::SourceLanguage TranslateSourceLanguage(Pglslang::EShSource source, EProfile profile)
{
#ifdef GLSLANG_WEB
    return Pspv::SourceLanguageESSL;
#endif

    switch (source) {
    case Pglslang::EShSourceGlsl:
        switch (profile) {
        case ENoProfile:
        case ECoreProfile:
        case ECompatibilityProfile:
            return Pspv::SourceLanguageGLSL;
        case EEsProfile:
            return Pspv::SourceLanguageESSL;
        default:
            return Pspv::SourceLanguageUnknown;
        }
    case Pglslang::EShSourceHlsl:
        return Pspv::SourceLanguageHLSL;
    default:
        return Pspv::SourceLanguageUnknown;
    }
}

// Translate glslang language (stage) to SPIR-V execution model.
Pspv::ExecutionModel TranslateExecutionModel(EShLanguage stage)
{
    switch (stage) {
    case EShLangVertex:           return Pspv::ExecutionModelVertex;
    case EShLangFragment:         return Pspv::ExecutionModelFragment;
    case EShLangCompute:          return Pspv::ExecutionModelGLCompute;
#ifndef GLSLANG_WEB
    case EShLangTessControl:      return Pspv::ExecutionModelTessellationControl;
    case EShLangTessEvaluation:   return Pspv::ExecutionModelTessellationEvaluation;
    case EShLangGeometry:         return Pspv::ExecutionModelGeometry;
    case EShLangRayGenNV:         return Pspv::ExecutionModelRayGenerationNV;
    case EShLangIntersectNV:      return Pspv::ExecutionModelIntersectionNV;
    case EShLangAnyHitNV:         return Pspv::ExecutionModelAnyHitNV;
    case EShLangClosestHitNV:     return Pspv::ExecutionModelClosestHitNV;
    case EShLangMissNV:           return Pspv::ExecutionModelMissNV;
    case EShLangCallableNV:       return Pspv::ExecutionModelCallableNV;
    case EShLangTaskNV:           return Pspv::ExecutionModelTaskNV;
    case EShLangMeshNV:           return Pspv::ExecutionModelMeshNV;
#endif
    default:
        assert(0);
        return Pspv::ExecutionModelFragment;
    }
}

// Translate glslang sampler type to SPIR-V dimensionality.
Pspv::Dim TranslateDimensionality(const Pglslang::TSampler& sampler)
{
    switch (sampler.dim) {
    case Pglslang::Esd1D:      return Pspv::Dim1D;
    case Pglslang::Esd2D:      return Pspv::Dim2D;
    case Pglslang::Esd3D:      return Pspv::Dim3D;
    case Pglslang::EsdCube:    return Pspv::DimCube;
    case Pglslang::EsdRect:    return Pspv::DimRect;
    case Pglslang::EsdBuffer:  return Pspv::DimBuffer;
    case Pglslang::EsdSubpass: return Pspv::DimSubpassData;
    default:
        assert(0);
        return Pspv::Dim2D;
    }
}

// Translate glslang precision to SPIR-V precision decorations.
Pspv::Decoration TranslatePrecisionDecoration(Pglslang::TPrecisionQualifier glslangPrecision)
{
    switch (glslangPrecision) {
    case Pglslang::EpqLow:    return Pspv::DecorationRelaxedPrecision;
    case Pglslang::EpqMedium: return Pspv::DecorationRelaxedPrecision;
    default:
        return Pspv::NoPrecision;
    }
}

// Translate glslang type to SPIR-V precision decorations.
Pspv::Decoration TranslatePrecisionDecoration(const Pglslang::TType& type)
{
    return TranslatePrecisionDecoration(type.getQualifier().precision);
}

// Translate glslang type to SPIR-V block decorations.
Pspv::Decoration TranslateBlockDecoration(const Pglslang::TType& type, bool useStorageBuffer)
{
    if (type.getBasicType() == Pglslang::EbtBlock) {
        switch (type.getQualifier().storage) {
        case Pglslang::EvqUniform:      return Pspv::DecorationBlock;
        case Pglslang::EvqBuffer:       return useStorageBuffer ? Pspv::DecorationBlock : Pspv::DecorationBufferBlock;
        case Pglslang::EvqVaryingIn:    return Pspv::DecorationBlock;
        case Pglslang::EvqVaryingOut:   return Pspv::DecorationBlock;
#ifndef GLSLANG_WEB
        case Pglslang::EvqPayloadNV:    return Pspv::DecorationBlock;
        case Pglslang::EvqPayloadInNV:  return Pspv::DecorationBlock;
        case Pglslang::EvqHitAttrNV:    return Pspv::DecorationBlock;
        case Pglslang::EvqCallableDataNV:   return Pspv::DecorationBlock;
        case Pglslang::EvqCallableDataInNV: return Pspv::DecorationBlock;
#endif
        default:
            assert(0);
            break;
        }
    }

    return Pspv::DecorationMax;
}

// Translate glslang type to SPIR-V memory decorations.
void TranslateMemoryDecoration(const Pglslang::TQualifier& qualifier, std::vector<Pspv::Decoration>& memory, bool useVulkanMemoryModel)
{
    if (!useVulkanMemoryModel) {
        if (qualifier.isCoherent())
            memory.push_back(Pspv::DecorationCoherent);
        if (qualifier.isVolatile()) {
            memory.push_back(Pspv::DecorationVolatile);
            memory.push_back(Pspv::DecorationCoherent);
        }
    }
    if (qualifier.isRestrict())
        memory.push_back(Pspv::DecorationRestrict);
    if (qualifier.isReadOnly())
        memory.push_back(Pspv::DecorationNonWritable);
    if (qualifier.isWriteOnly())
       memory.push_back(Pspv::DecorationNonReadable);
}

// Translate glslang type to SPIR-V layout decorations.
Pspv::Decoration TranslateLayoutDecoration(const Pglslang::TType& type, Pglslang::TLayoutMatrix matrixLayout)
{
    if (type.isMatrix()) {
        switch (matrixLayout) {
        case Pglslang::ElmRowMajor:
            return Pspv::DecorationRowMajor;
        case Pglslang::ElmColumnMajor:
            return Pspv::DecorationColMajor;
        default:
            // opaque layouts don't need a majorness
            return Pspv::DecorationMax;
        }
    } else {
        switch (type.getBasicType()) {
        default:
            return Pspv::DecorationMax;
            break;
        case Pglslang::EbtBlock:
            switch (type.getQualifier().storage) {
            case Pglslang::EvqUniform:
            case Pglslang::EvqBuffer:
                switch (type.getQualifier().layoutPacking) {
                case Pglslang::ElpShared:  return Pspv::DecorationGLSLShared;
                case Pglslang::ElpPacked:  return Pspv::DecorationGLSLPacked;
                default:
                    return Pspv::DecorationMax;
                }
            case Pglslang::EvqVaryingIn:
            case Pglslang::EvqVaryingOut:
                if (type.getQualifier().isTaskMemory()) {
                    switch (type.getQualifier().layoutPacking) {
                    case Pglslang::ElpShared:  return Pspv::DecorationGLSLShared;
                    case Pglslang::ElpPacked:  return Pspv::DecorationGLSLPacked;
                    default: break;
                    }
                } else {
                    assert(type.getQualifier().layoutPacking == Pglslang::ElpNone);
                }
                return Pspv::DecorationMax;
#ifndef GLSLANG_WEB
            case Pglslang::EvqPayloadNV:
            case Pglslang::EvqPayloadInNV:
            case Pglslang::EvqHitAttrNV:
            case Pglslang::EvqCallableDataNV:
            case Pglslang::EvqCallableDataInNV:
                return Pspv::DecorationMax;
#endif
            default:
                assert(0);
                return Pspv::DecorationMax;
            }
        }
    }
}

// Translate glslang type to SPIR-V interpolation decorations.
// Returns Pspv::DecorationMax when no decoration
// should be applied.
Pspv::Decoration TGlslangToSpvTraverser::TranslateInterpolationDecoration(const Pglslang::TQualifier& qualifier)
{
    if (qualifier.smooth)
        // Smooth decoration doesn't exist in SPIR-V 1.0
        return Pspv::DecorationMax;
    else if (qualifier.isNonPerspective())
        return Pspv::DecorationNoPerspective;
    else if (qualifier.flat)
        return Pspv::DecorationFlat;
    else if (qualifier.isExplicitInterpolation()) {
        builder.addExtension(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return Pspv::DecorationExplicitInterpAMD;
    }
    else
        return Pspv::DecorationMax;
}

// Translate glslang type to SPIR-V auxiliary storage decorations.
// Returns Pspv::DecorationMax when no decoration
// should be applied.
Pspv::Decoration TGlslangToSpvTraverser::TranslateAuxiliaryStorageDecoration(const Pglslang::TQualifier& qualifier)
{
    if (qualifier.centroid)
        return Pspv::DecorationCentroid;
#ifndef GLSLANG_WEB
    else if (qualifier.patch)
        return Pspv::DecorationPatch;
    else if (qualifier.sample) {
        builder.addCapability(Pspv::CapabilitySampleRateShading);
        return Pspv::DecorationSample;
    }
#endif

    return Pspv::DecorationMax;
}

// If glslang type is invariant, return SPIR-V invariant decoration.
Pspv::Decoration TranslateInvariantDecoration(const Pglslang::TQualifier& qualifier)
{
    if (qualifier.invariant)
        return Pspv::DecorationInvariant;
    else
        return Pspv::DecorationMax;
}

// If glslang type is noContraction, return SPIR-V NoContraction decoration.
Pspv::Decoration TranslateNoContractionDecoration(const Pglslang::TQualifier& qualifier)
{
#ifndef GLSLANG_WEB
    if (qualifier.isNoContraction())
        return Pspv::DecorationNoContraction;
    else
#endif
        return Pspv::DecorationMax;
}

// If glslang type is nonUniform, return SPIR-V NonUniform decoration.
Pspv::Decoration TGlslangToSpvTraverser::TranslateNonUniformDecoration(const Pglslang::TQualifier& qualifier)
{
#ifndef GLSLANG_WEB
    if (qualifier.isNonUniform()) {
        builder.addIncorporatedExtension("SPV_EXT_descriptor_indexing", Pspv::Spv_1_5);
        builder.addCapability(Pspv::CapabilityShaderNonUniformEXT);
        return Pspv::DecorationNonUniformEXT;
    } else
#endif
        return Pspv::DecorationMax;
}

Pspv::MemoryAccessMask TGlslangToSpvTraverser::TranslateMemoryAccess(
    const Pspv::Builder::AccessChain::CoherentFlags &coherentFlags)
{
    Pspv::MemoryAccessMask mask = Pspv::MemoryAccessMaskNone;

#ifndef GLSLANG_WEB
    if (!glslangIntermediate->usingVulkanMemoryModel() || coherentFlags.isImage)
        return mask;

    if (coherentFlags.volatil ||
        coherentFlags.coherent ||
        coherentFlags.devicecoherent ||
        coherentFlags.queuefamilycoherent ||
        coherentFlags.workgroupcoherent ||
        coherentFlags.subgroupcoherent) {
        mask = mask | Pspv::MemoryAccessMakePointerAvailableKHRMask |
                      Pspv::MemoryAccessMakePointerVisibleKHRMask;
    }
    if (coherentFlags.nonprivate) {
        mask = mask | Pspv::MemoryAccessNonPrivatePointerKHRMask;
    }
    if (coherentFlags.volatil) {
        mask = mask | Pspv::MemoryAccessVolatileMask;
    }
    if (mask != Pspv::MemoryAccessMaskNone) {
        builder.addCapability(Pspv::CapabilityVulkanMemoryModelKHR);
    }
#endif

    return mask;
}

Pspv::ImageOperandsMask TGlslangToSpvTraverser::TranslateImageOperands(
    const Pspv::Builder::AccessChain::CoherentFlags &coherentFlags)
{
    Pspv::ImageOperandsMask mask = Pspv::ImageOperandsMaskNone;

#ifndef GLSLANG_WEB
    if (!glslangIntermediate->usingVulkanMemoryModel())
        return mask;

    if (coherentFlags.volatil ||
        coherentFlags.coherent ||
        coherentFlags.devicecoherent ||
        coherentFlags.queuefamilycoherent ||
        coherentFlags.workgroupcoherent ||
        coherentFlags.subgroupcoherent) {
        mask = mask | Pspv::ImageOperandsMakeTexelAvailableKHRMask |
                      Pspv::ImageOperandsMakeTexelVisibleKHRMask;
    }
    if (coherentFlags.nonprivate) {
        mask = mask | Pspv::ImageOperandsNonPrivateTexelKHRMask;
    }
    if (coherentFlags.volatil) {
        mask = mask | Pspv::ImageOperandsVolatileTexelKHRMask;
    }
    if (mask != Pspv::ImageOperandsMaskNone) {
        builder.addCapability(Pspv::CapabilityVulkanMemoryModelKHR);
    }
#endif

    return mask;
}

Pspv::Builder::AccessChain::CoherentFlags TGlslangToSpvTraverser::TranslateCoherent(const Pglslang::TType& type)
{
    Pspv::Builder::AccessChain::CoherentFlags flags = {};
#ifndef GLSLANG_WEB
    flags.coherent = type.getQualifier().coherent;
    flags.devicecoherent = type.getQualifier().devicecoherent;
    flags.queuefamilycoherent = type.getQualifier().queuefamilycoherent;
    // shared variables are implicitly workgroupcoherent in GLSL.
    flags.workgroupcoherent = type.getQualifier().workgroupcoherent ||
                              type.getQualifier().storage == Pglslang::EvqShared;
    flags.subgroupcoherent = type.getQualifier().subgroupcoherent;
    flags.volatil = type.getQualifier().volatil;
    // *coherent variables are implicitly nonprivate in GLSL
    flags.nonprivate = type.getQualifier().nonprivate ||
                       flags.subgroupcoherent ||
                       flags.workgroupcoherent ||
                       flags.queuefamilycoherent ||
                       flags.devicecoherent ||
                       flags.coherent ||
                       flags.volatil;
    flags.isImage = type.getBasicType() == Pglslang::EbtSampler;
#endif
    return flags;
}

Pspv::Scope TGlslangToSpvTraverser::TranslateMemoryScope(
    const Pspv::Builder::AccessChain::CoherentFlags &coherentFlags)
{
    Pspv::Scope scope = Pspv::ScopeMax;

#ifndef GLSLANG_WEB
    if (coherentFlags.volatil || coherentFlags.coherent) {
        // coherent defaults to Device scope in the old model, QueueFamilyKHR scope in the new model
        scope = glslangIntermediate->usingVulkanMemoryModel() ? Pspv::ScopeQueueFamilyKHR : Pspv::ScopeDevice;
    } else if (coherentFlags.devicecoherent) {
        scope = Pspv::ScopeDevice;
    } else if (coherentFlags.queuefamilycoherent) {
        scope = Pspv::ScopeQueueFamilyKHR;
    } else if (coherentFlags.workgroupcoherent) {
        scope = Pspv::ScopeWorkgroup;
    } else if (coherentFlags.subgroupcoherent) {
        scope = Pspv::ScopeSubgroup;
    }
    if (glslangIntermediate->usingVulkanMemoryModel() && scope == Pspv::ScopeDevice) {
        builder.addCapability(Pspv::CapabilityVulkanMemoryModelDeviceScopeKHR);
    }
#endif

    return scope;
}

// Translate a glslang built-in variable to a SPIR-V built in decoration.  Also generate
// associated capabilities when required.  For some built-in variables, a capability
// is generated only when using the variable in an executable instruction, but not when
// just declaring a struct member variable with it.  This is true for PointSize,
// ClipDistance, and CullDistance.
Pspv::BuiltIn TGlslangToSpvTraverser::TranslateBuiltInDecoration(Pglslang::TBuiltInVariable builtIn, bool memberDeclaration)
{
    switch (builtIn) {
    case Pglslang::EbvPointSize:
#ifndef GLSLANG_WEB
        // Defer adding the capability until the built-in is actually used.
        if (! memberDeclaration) {
            switch (glslangIntermediate->getStage()) {
            case EShLangGeometry:
                builder.addCapability(Pspv::CapabilityGeometryPointSize);
                break;
            case EShLangTessControl:
            case EShLangTessEvaluation:
                builder.addCapability(Pspv::CapabilityTessellationPointSize);
                break;
            default:
                break;
            }
        }
#endif
        return Pspv::BuiltInPointSize;

    case Pglslang::EbvPosition:             return Pspv::BuiltInPosition;
    case Pglslang::EbvVertexId:             return Pspv::BuiltInVertexId;
    case Pglslang::EbvInstanceId:           return Pspv::BuiltInInstanceId;
    case Pglslang::EbvVertexIndex:          return Pspv::BuiltInVertexIndex;
    case Pglslang::EbvInstanceIndex:        return Pspv::BuiltInInstanceIndex;

    case Pglslang::EbvFragCoord:            return Pspv::BuiltInFragCoord;
    case Pglslang::EbvPointCoord:           return Pspv::BuiltInPointCoord;
    case Pglslang::EbvFace:                 return Pspv::BuiltInFrontFacing;
    case Pglslang::EbvFragDepth:            return Pspv::BuiltInFragDepth;

    case Pglslang::EbvNumWorkGroups:        return Pspv::BuiltInNumWorkgroups;
    case Pglslang::EbvWorkGroupSize:        return Pspv::BuiltInWorkgroupSize;
    case Pglslang::EbvWorkGroupId:          return Pspv::BuiltInWorkgroupId;
    case Pglslang::EbvLocalInvocationId:    return Pspv::BuiltInLocalInvocationId;
    case Pglslang::EbvLocalInvocationIndex: return Pspv::BuiltInLocalInvocationIndex;
    case Pglslang::EbvGlobalInvocationId:   return Pspv::BuiltInGlobalInvocationId;

#ifndef GLSLANG_WEB
    // These *Distance capabilities logically belong here, but if the member is declared and
    // then never used, consumers of SPIR-V prefer the capability not be declared.
    // They are now generated when used, rather than here when declared.
    // Potentially, the specification should be more clear what the minimum
    // use needed is to trigger the capability.
    //
    case Pglslang::EbvClipDistance:
        if (!memberDeclaration)
            builder.addCapability(Pspv::CapabilityClipDistance);
        return Pspv::BuiltInClipDistance;

    case Pglslang::EbvCullDistance:
        if (!memberDeclaration)
            builder.addCapability(Pspv::CapabilityCullDistance);
        return Pspv::BuiltInCullDistance;

    case Pglslang::EbvViewportIndex:
        builder.addCapability(Pspv::CapabilityMultiViewport);
        if (glslangIntermediate->getStage() == EShLangVertex ||
            glslangIntermediate->getStage() == EShLangTessControl ||
            glslangIntermediate->getStage() == EShLangTessEvaluation) {

            builder.addIncorporatedExtension(Pspv::E_SPV_EXT_shader_viewport_index_layer, Pspv::Spv_1_5);
            builder.addCapability(Pspv::CapabilityShaderViewportIndexLayerEXT);
        }
        return Pspv::BuiltInViewportIndex;

    case Pglslang::EbvSampleId:
        builder.addCapability(Pspv::CapabilitySampleRateShading);
        return Pspv::BuiltInSampleId;

    case Pglslang::EbvSamplePosition:
        builder.addCapability(Pspv::CapabilitySampleRateShading);
        return Pspv::BuiltInSamplePosition;

    case Pglslang::EbvSampleMask:
        return Pspv::BuiltInSampleMask;

    case Pglslang::EbvLayer:
        if (glslangIntermediate->getStage() == EShLangMeshNV) {
            return Pspv::BuiltInLayer;
        }
        builder.addCapability(Pspv::CapabilityGeometry);
        if (glslangIntermediate->getStage() == EShLangVertex ||
            glslangIntermediate->getStage() == EShLangTessControl ||
            glslangIntermediate->getStage() == EShLangTessEvaluation) {

            builder.addIncorporatedExtension(Pspv::E_SPV_EXT_shader_viewport_index_layer, Pspv::Spv_1_5);
            builder.addCapability(Pspv::CapabilityShaderViewportIndexLayerEXT);
        }
        return Pspv::BuiltInLayer;

    case Pglslang::EbvBaseVertex:
        builder.addIncorporatedExtension(Pspv::E_SPV_KHR_shader_draw_parameters, Pspv::Spv_1_3);
        builder.addCapability(Pspv::CapabilityDrawParameters);
        return Pspv::BuiltInBaseVertex;

    case Pglslang::EbvBaseInstance:
        builder.addIncorporatedExtension(Pspv::E_SPV_KHR_shader_draw_parameters, Pspv::Spv_1_3);
        builder.addCapability(Pspv::CapabilityDrawParameters);
        return Pspv::BuiltInBaseInstance;

    case Pglslang::EbvDrawId:
        builder.addIncorporatedExtension(Pspv::E_SPV_KHR_shader_draw_parameters, Pspv::Spv_1_3);
        builder.addCapability(Pspv::CapabilityDrawParameters);
        return Pspv::BuiltInDrawIndex;

    case Pglslang::EbvPrimitiveId:
        if (glslangIntermediate->getStage() == EShLangFragment)
            builder.addCapability(Pspv::CapabilityGeometry);
        return Pspv::BuiltInPrimitiveId;

    case Pglslang::EbvFragStencilRef:
        builder.addExtension(Pspv::E_SPV_EXT_shader_stencil_export);
        builder.addCapability(Pspv::CapabilityStencilExportEXT);
        return Pspv::BuiltInFragStencilRefEXT;

    case Pglslang::EbvInvocationId:         return Pspv::BuiltInInvocationId;
    case Pglslang::EbvTessLevelInner:       return Pspv::BuiltInTessLevelInner;
    case Pglslang::EbvTessLevelOuter:       return Pspv::BuiltInTessLevelOuter;
    case Pglslang::EbvTessCoord:            return Pspv::BuiltInTessCoord;
    case Pglslang::EbvPatchVertices:        return Pspv::BuiltInPatchVertices;
    case Pglslang::EbvHelperInvocation:     return Pspv::BuiltInHelperInvocation;

    case Pglslang::EbvSubGroupSize:
        builder.addExtension(Pspv::E_SPV_KHR_shader_ballot);
        builder.addCapability(Pspv::CapabilitySubgroupBallotKHR);
        return Pspv::BuiltInSubgroupSize;

    case Pglslang::EbvSubGroupInvocation:
        builder.addExtension(Pspv::E_SPV_KHR_shader_ballot);
        builder.addCapability(Pspv::CapabilitySubgroupBallotKHR);
        return Pspv::BuiltInSubgroupLocalInvocationId;

    case Pglslang::EbvSubGroupEqMask:
        builder.addExtension(Pspv::E_SPV_KHR_shader_ballot);
        builder.addCapability(Pspv::CapabilitySubgroupBallotKHR);
        return Pspv::BuiltInSubgroupEqMask;

    case Pglslang::EbvSubGroupGeMask:
        builder.addExtension(Pspv::E_SPV_KHR_shader_ballot);
        builder.addCapability(Pspv::CapabilitySubgroupBallotKHR);
        return Pspv::BuiltInSubgroupGeMask;

    case Pglslang::EbvSubGroupGtMask:
        builder.addExtension(Pspv::E_SPV_KHR_shader_ballot);
        builder.addCapability(Pspv::CapabilitySubgroupBallotKHR);
        return Pspv::BuiltInSubgroupGtMask;

    case Pglslang::EbvSubGroupLeMask:
        builder.addExtension(Pspv::E_SPV_KHR_shader_ballot);
        builder.addCapability(Pspv::CapabilitySubgroupBallotKHR);
        return Pspv::BuiltInSubgroupLeMask;

    case Pglslang::EbvSubGroupLtMask:
        builder.addExtension(Pspv::E_SPV_KHR_shader_ballot);
        builder.addCapability(Pspv::CapabilitySubgroupBallotKHR);
        return Pspv::BuiltInSubgroupLtMask;

    case Pglslang::EbvNumSubgroups:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        return Pspv::BuiltInNumSubgroups;

    case Pglslang::EbvSubgroupID:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        return Pspv::BuiltInSubgroupId;

    case Pglslang::EbvSubgroupSize2:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        return Pspv::BuiltInSubgroupSize;

    case Pglslang::EbvSubgroupInvocation2:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        return Pspv::BuiltInSubgroupLocalInvocationId;

    case Pglslang::EbvSubgroupEqMask2:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformBallot);
        return Pspv::BuiltInSubgroupEqMask;

    case Pglslang::EbvSubgroupGeMask2:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformBallot);
        return Pspv::BuiltInSubgroupGeMask;

    case Pglslang::EbvSubgroupGtMask2:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformBallot);
        return Pspv::BuiltInSubgroupGtMask;

    case Pglslang::EbvSubgroupLeMask2:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformBallot);
        return Pspv::BuiltInSubgroupLeMask;

    case Pglslang::EbvSubgroupLtMask2:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformBallot);
        return Pspv::BuiltInSubgroupLtMask;

    case Pglslang::EbvBaryCoordNoPersp:
        builder.addExtension(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return Pspv::BuiltInBaryCoordNoPerspAMD;

    case Pglslang::EbvBaryCoordNoPerspCentroid:
        builder.addExtension(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return Pspv::BuiltInBaryCoordNoPerspCentroidAMD;

    case Pglslang::EbvBaryCoordNoPerspSample:
        builder.addExtension(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return Pspv::BuiltInBaryCoordNoPerspSampleAMD;

    case Pglslang::EbvBaryCoordSmooth:
        builder.addExtension(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return Pspv::BuiltInBaryCoordSmoothAMD;

    case Pglslang::EbvBaryCoordSmoothCentroid:
        builder.addExtension(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return Pspv::BuiltInBaryCoordSmoothCentroidAMD;

    case Pglslang::EbvBaryCoordSmoothSample:
        builder.addExtension(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return Pspv::BuiltInBaryCoordSmoothSampleAMD;

    case Pglslang::EbvBaryCoordPullModel:
        builder.addExtension(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        return Pspv::BuiltInBaryCoordPullModelAMD;

    case Pglslang::EbvDeviceIndex:
        builder.addIncorporatedExtension(Pspv::E_SPV_KHR_device_group, Pspv::Spv_1_3);
        builder.addCapability(Pspv::CapabilityDeviceGroup);
        return Pspv::BuiltInDeviceIndex;

    case Pglslang::EbvViewIndex:
        builder.addIncorporatedExtension(Pspv::E_SPV_KHR_multiview, Pspv::Spv_1_3);
        builder.addCapability(Pspv::CapabilityMultiView);
        return Pspv::BuiltInViewIndex;

    case Pglslang::EbvFragSizeEXT:
        builder.addExtension(Pspv::E_SPV_EXT_fragment_invocation_density);
        builder.addCapability(Pspv::CapabilityFragmentDensityEXT);
        return Pspv::BuiltInFragSizeEXT;

    case Pglslang::EbvFragInvocationCountEXT:
        builder.addExtension(Pspv::E_SPV_EXT_fragment_invocation_density);
        builder.addCapability(Pspv::CapabilityFragmentDensityEXT);
        return Pspv::BuiltInFragInvocationCountEXT;

    case Pglslang::EbvViewportMaskNV:
        if (!memberDeclaration) {
            builder.addExtension(Pspv::E_SPV_NV_viewport_array2);
            builder.addCapability(Pspv::CapabilityShaderViewportMaskNV);
        }
        return Pspv::BuiltInViewportMaskNV;
    case Pglslang::EbvSecondaryPositionNV:
        if (!memberDeclaration) {
            builder.addExtension(Pspv::E_SPV_NV_stereo_view_rendering);
            builder.addCapability(Pspv::CapabilityShaderStereoViewNV);
        }
        return Pspv::BuiltInSecondaryPositionNV;
    case Pglslang::EbvSecondaryViewportMaskNV:
        if (!memberDeclaration) {
            builder.addExtension(Pspv::E_SPV_NV_stereo_view_rendering);
            builder.addCapability(Pspv::CapabilityShaderStereoViewNV);
        }
        return Pspv::BuiltInSecondaryViewportMaskNV;
    case Pglslang::EbvPositionPerViewNV:
        if (!memberDeclaration) {
            builder.addExtension(Pspv::E_SPV_NVX_multiview_per_view_attributes);
            builder.addCapability(Pspv::CapabilityPerViewAttributesNV);
        }
        return Pspv::BuiltInPositionPerViewNV;
    case Pglslang::EbvViewportMaskPerViewNV:
        if (!memberDeclaration) {
            builder.addExtension(Pspv::E_SPV_NVX_multiview_per_view_attributes);
            builder.addCapability(Pspv::CapabilityPerViewAttributesNV);
        }
        return Pspv::BuiltInViewportMaskPerViewNV;
    case Pglslang::EbvFragFullyCoveredNV:
        builder.addExtension(Pspv::E_SPV_EXT_fragment_fully_covered);
        builder.addCapability(Pspv::CapabilityFragmentFullyCoveredEXT);
        return Pspv::BuiltInFullyCoveredEXT;
    case Pglslang::EbvFragmentSizeNV:
        builder.addExtension(Pspv::E_SPV_NV_shading_rate);
        builder.addCapability(Pspv::CapabilityShadingRateNV);
        return Pspv::BuiltInFragmentSizeNV;
    case Pglslang::EbvInvocationsPerPixelNV:
        builder.addExtension(Pspv::E_SPV_NV_shading_rate);
        builder.addCapability(Pspv::CapabilityShadingRateNV);
        return Pspv::BuiltInInvocationsPerPixelNV;

    // ray tracing
    case Pglslang::EbvLaunchIdNV:
        return Pspv::BuiltInLaunchIdNV;
    case Pglslang::EbvLaunchSizeNV:
        return Pspv::BuiltInLaunchSizeNV;
    case Pglslang::EbvWorldRayOriginNV:
        return Pspv::BuiltInWorldRayOriginNV;
    case Pglslang::EbvWorldRayDirectionNV:
        return Pspv::BuiltInWorldRayDirectionNV;
    case Pglslang::EbvObjectRayOriginNV:
        return Pspv::BuiltInObjectRayOriginNV;
    case Pglslang::EbvObjectRayDirectionNV:
        return Pspv::BuiltInObjectRayDirectionNV;
    case Pglslang::EbvRayTminNV:
        return Pspv::BuiltInRayTminNV;
    case Pglslang::EbvRayTmaxNV:
        return Pspv::BuiltInRayTmaxNV;
    case Pglslang::EbvInstanceCustomIndexNV:
        return Pspv::BuiltInInstanceCustomIndexNV;
    case Pglslang::EbvHitTNV:
        return Pspv::BuiltInHitTNV;
    case Pglslang::EbvHitKindNV:
        return Pspv::BuiltInHitKindNV;
    case Pglslang::EbvObjectToWorldNV:
        return Pspv::BuiltInObjectToWorldNV;
    case Pglslang::EbvWorldToObjectNV:
        return Pspv::BuiltInWorldToObjectNV;
    case Pglslang::EbvIncomingRayFlagsNV:
        return Pspv::BuiltInIncomingRayFlagsNV;

    // barycentrics
    case Pglslang::EbvBaryCoordNV:
        builder.addExtension(Pspv::E_SPV_NV_fragment_shader_barycentric);
        builder.addCapability(Pspv::CapabilityFragmentBarycentricNV);
        return Pspv::BuiltInBaryCoordNV;
    case Pglslang::EbvBaryCoordNoPerspNV:
        builder.addExtension(Pspv::E_SPV_NV_fragment_shader_barycentric);
        builder.addCapability(Pspv::CapabilityFragmentBarycentricNV);
        return Pspv::BuiltInBaryCoordNoPerspNV;

    // mesh shaders
    case Pglslang::EbvTaskCountNV:
        return Pspv::BuiltInTaskCountNV;
    case Pglslang::EbvPrimitiveCountNV:
        return Pspv::BuiltInPrimitiveCountNV;
    case Pglslang::EbvPrimitiveIndicesNV:
        return Pspv::BuiltInPrimitiveIndicesNV;
    case Pglslang::EbvClipDistancePerViewNV:
        return Pspv::BuiltInClipDistancePerViewNV;
    case Pglslang::EbvCullDistancePerViewNV:
        return Pspv::BuiltInCullDistancePerViewNV;
    case Pglslang::EbvLayerPerViewNV:
        return Pspv::BuiltInLayerPerViewNV;
    case Pglslang::EbvMeshViewCountNV:
        return Pspv::BuiltInMeshViewCountNV;
    case Pglslang::EbvMeshViewIndicesNV:
        return Pspv::BuiltInMeshViewIndicesNV;

    // sm builtins
    case Pglslang::EbvWarpsPerSM:
        builder.addExtension(Pspv::E_SPV_NV_shader_sm_builtins);
        builder.addCapability(Pspv::CapabilityShaderSMBuiltinsNV);
        return Pspv::BuiltInWarpsPerSMNV;
    case Pglslang::EbvSMCount:
        builder.addExtension(Pspv::E_SPV_NV_shader_sm_builtins);
        builder.addCapability(Pspv::CapabilityShaderSMBuiltinsNV);
        return Pspv::BuiltInSMCountNV;
    case Pglslang::EbvWarpID:
        builder.addExtension(Pspv::E_SPV_NV_shader_sm_builtins);
        builder.addCapability(Pspv::CapabilityShaderSMBuiltinsNV);
        return Pspv::BuiltInWarpIDNV;
    case Pglslang::EbvSMID:
        builder.addExtension(Pspv::E_SPV_NV_shader_sm_builtins);
        builder.addCapability(Pspv::CapabilityShaderSMBuiltinsNV);
        return Pspv::BuiltInSMIDNV;
#endif

    default:
        return Pspv::BuiltInMax;
    }
}

// Translate glslang image layout format to SPIR-V image format.
Pspv::ImageFormat TGlslangToSpvTraverser::TranslateImageFormat(const Pglslang::TType& type)
{
    assert(type.getBasicType() == Pglslang::EbtSampler);

#ifdef GLSLANG_WEB
    return Pspv::ImageFormatUnknown;
#endif

    // Check for capabilities
    switch (type.getQualifier().getFormat()) {
    case Pglslang::ElfRg32f:
    case Pglslang::ElfRg16f:
    case Pglslang::ElfR11fG11fB10f:
    case Pglslang::ElfR16f:
    case Pglslang::ElfRgba16:
    case Pglslang::ElfRgb10A2:
    case Pglslang::ElfRg16:
    case Pglslang::ElfRg8:
    case Pglslang::ElfR16:
    case Pglslang::ElfR8:
    case Pglslang::ElfRgba16Snorm:
    case Pglslang::ElfRg16Snorm:
    case Pglslang::ElfRg8Snorm:
    case Pglslang::ElfR16Snorm:
    case Pglslang::ElfR8Snorm:

    case Pglslang::ElfRg32i:
    case Pglslang::ElfRg16i:
    case Pglslang::ElfRg8i:
    case Pglslang::ElfR16i:
    case Pglslang::ElfR8i:

    case Pglslang::ElfRgb10a2ui:
    case Pglslang::ElfRg32ui:
    case Pglslang::ElfRg16ui:
    case Pglslang::ElfRg8ui:
    case Pglslang::ElfR16ui:
    case Pglslang::ElfR8ui:
        builder.addCapability(Pspv::CapabilityStorageImageExtendedFormats);
        break;

    default:
        break;
    }

    // do the translation
    switch (type.getQualifier().getFormat()) {
    case Pglslang::ElfNone:          return Pspv::ImageFormatUnknown;
    case Pglslang::ElfRgba32f:       return Pspv::ImageFormatRgba32f;
    case Pglslang::ElfRgba16f:       return Pspv::ImageFormatRgba16f;
    case Pglslang::ElfR32f:          return Pspv::ImageFormatR32f;
    case Pglslang::ElfRgba8:         return Pspv::ImageFormatRgba8;
    case Pglslang::ElfRgba8Snorm:    return Pspv::ImageFormatRgba8Snorm;
    case Pglslang::ElfRg32f:         return Pspv::ImageFormatRg32f;
    case Pglslang::ElfRg16f:         return Pspv::ImageFormatRg16f;
    case Pglslang::ElfR11fG11fB10f:  return Pspv::ImageFormatR11fG11fB10f;
    case Pglslang::ElfR16f:          return Pspv::ImageFormatR16f;
    case Pglslang::ElfRgba16:        return Pspv::ImageFormatRgba16;
    case Pglslang::ElfRgb10A2:       return Pspv::ImageFormatRgb10A2;
    case Pglslang::ElfRg16:          return Pspv::ImageFormatRg16;
    case Pglslang::ElfRg8:           return Pspv::ImageFormatRg8;
    case Pglslang::ElfR16:           return Pspv::ImageFormatR16;
    case Pglslang::ElfR8:            return Pspv::ImageFormatR8;
    case Pglslang::ElfRgba16Snorm:   return Pspv::ImageFormatRgba16Snorm;
    case Pglslang::ElfRg16Snorm:     return Pspv::ImageFormatRg16Snorm;
    case Pglslang::ElfRg8Snorm:      return Pspv::ImageFormatRg8Snorm;
    case Pglslang::ElfR16Snorm:      return Pspv::ImageFormatR16Snorm;
    case Pglslang::ElfR8Snorm:       return Pspv::ImageFormatR8Snorm;
    case Pglslang::ElfRgba32i:       return Pspv::ImageFormatRgba32i;
    case Pglslang::ElfRgba16i:       return Pspv::ImageFormatRgba16i;
    case Pglslang::ElfRgba8i:        return Pspv::ImageFormatRgba8i;
    case Pglslang::ElfR32i:          return Pspv::ImageFormatR32i;
    case Pglslang::ElfRg32i:         return Pspv::ImageFormatRg32i;
    case Pglslang::ElfRg16i:         return Pspv::ImageFormatRg16i;
    case Pglslang::ElfRg8i:          return Pspv::ImageFormatRg8i;
    case Pglslang::ElfR16i:          return Pspv::ImageFormatR16i;
    case Pglslang::ElfR8i:           return Pspv::ImageFormatR8i;
    case Pglslang::ElfRgba32ui:      return Pspv::ImageFormatRgba32ui;
    case Pglslang::ElfRgba16ui:      return Pspv::ImageFormatRgba16ui;
    case Pglslang::ElfRgba8ui:       return Pspv::ImageFormatRgba8ui;
    case Pglslang::ElfR32ui:         return Pspv::ImageFormatR32ui;
    case Pglslang::ElfRg32ui:        return Pspv::ImageFormatRg32ui;
    case Pglslang::ElfRg16ui:        return Pspv::ImageFormatRg16ui;
    case Pglslang::ElfRgb10a2ui:     return Pspv::ImageFormatRgb10a2ui;
    case Pglslang::ElfRg8ui:         return Pspv::ImageFormatRg8ui;
    case Pglslang::ElfR16ui:         return Pspv::ImageFormatR16ui;
    case Pglslang::ElfR8ui:          return Pspv::ImageFormatR8ui;
    default:                        return Pspv::ImageFormatMax;
    }
}

Pspv::SelectionControlMask TGlslangToSpvTraverser::TranslateSelectionControl(const Pglslang::TIntermSelection& selectionNode) const
{
    if (selectionNode.getFlatten())
        return Pspv::SelectionControlFlattenMask;
    if (selectionNode.getDontFlatten())
        return Pspv::SelectionControlDontFlattenMask;
    return Pspv::SelectionControlMaskNone;
}

Pspv::SelectionControlMask TGlslangToSpvTraverser::TranslateSwitchControl(const Pglslang::TIntermSwitch& switchNode) const
{
    if (switchNode.getFlatten())
        return Pspv::SelectionControlFlattenMask;
    if (switchNode.getDontFlatten())
        return Pspv::SelectionControlDontFlattenMask;
    return Pspv::SelectionControlMaskNone;
}

// return a non-0 dependency if the dependency argument must be set
Pspv::LoopControlMask TGlslangToSpvTraverser::TranslateLoopControl(const Pglslang::TIntermLoop& loopNode,
    std::vector<unsigned int>& operands) const
{
    Pspv::LoopControlMask control = Pspv::LoopControlMaskNone;

    if (loopNode.getDontUnroll())
        control = control | Pspv::LoopControlDontUnrollMask;
    if (loopNode.getUnroll())
        control = control | Pspv::LoopControlUnrollMask;
    if (unsigned(loopNode.getLoopDependency()) == Pglslang::TIntermLoop::dependencyInfinite)
        control = control | Pspv::LoopControlDependencyInfiniteMask;
    else if (loopNode.getLoopDependency() > 0) {
        control = control | Pspv::LoopControlDependencyLengthMask;
        operands.push_back((unsigned int)loopNode.getLoopDependency());
    }
    if (glslangIntermediate->getSpv().spv >= Pglslang::EShTargetSpv_1_4) {
        if (loopNode.getMinIterations() > 0) {
            control = control | Pspv::LoopControlMinIterationsMask;
            operands.push_back(loopNode.getMinIterations());
        }
        if (loopNode.getMaxIterations() < Pglslang::TIntermLoop::iterationsInfinite) {
            control = control | Pspv::LoopControlMaxIterationsMask;
            operands.push_back(loopNode.getMaxIterations());
        }
        if (loopNode.getIterationMultiple() > 1) {
            control = control | Pspv::LoopControlIterationMultipleMask;
            operands.push_back(loopNode.getIterationMultiple());
        }
        if (loopNode.getPeelCount() > 0) {
            control = control | Pspv::LoopControlPeelCountMask;
            operands.push_back(loopNode.getPeelCount());
        }
        if (loopNode.getPartialCount() > 0) {
            control = control | Pspv::LoopControlPartialCountMask;
            operands.push_back(loopNode.getPartialCount());
        }
    }

    return control;
}

// Translate glslang type to SPIR-V storage class.
Pspv::StorageClass TGlslangToSpvTraverser::TranslateStorageClass(const Pglslang::TType& type)
{
    if (type.getQualifier().isPipeInput())
        return Pspv::StorageClassInput;
    if (type.getQualifier().isPipeOutput())
        return Pspv::StorageClassOutput;

    if (glslangIntermediate->getSource() != Pglslang::EShSourceHlsl ||
            type.getQualifier().storage == Pglslang::EvqUniform) {
        if (type.isAtomic())
            return Pspv::StorageClassAtomicCounter;
        if (type.containsOpaque())
            return Pspv::StorageClassUniformConstant;
    }

    if (type.getQualifier().isUniformOrBuffer() &&
        type.getQualifier().isShaderRecordNV()) {
        return Pspv::StorageClassShaderRecordBufferNV;
    }

    if (glslangIntermediate->usingStorageBuffer() && type.getQualifier().storage == Pglslang::EvqBuffer) {
        builder.addIncorporatedExtension(Pspv::E_SPV_KHR_storage_buffer_storage_class, Pspv::Spv_1_3);
        return Pspv::StorageClassStorageBuffer;
    }

    if (type.getQualifier().isUniformOrBuffer()) {
        if (type.getQualifier().isPushConstant())
            return Pspv::StorageClassPushConstant;
        if (type.getBasicType() == Pglslang::EbtBlock)
            return Pspv::StorageClassUniform;
        return Pspv::StorageClassUniformConstant;
    }

    switch (type.getQualifier().storage) {
    case Pglslang::EvqGlobal:        return Pspv::StorageClassPrivate;
    case Pglslang::EvqConstReadOnly: return Pspv::StorageClassFunction;
    case Pglslang::EvqTemporary:     return Pspv::StorageClassFunction;
    case Pglslang::EvqShared:           return Pspv::StorageClassWorkgroup;
#ifndef GLSLANG_WEB
    case Pglslang::EvqPayloadNV:        return Pspv::StorageClassRayPayloadNV;
    case Pglslang::EvqPayloadInNV:      return Pspv::StorageClassIncomingRayPayloadNV;
    case Pglslang::EvqHitAttrNV:        return Pspv::StorageClassHitAttributeNV;
    case Pglslang::EvqCallableDataNV:   return Pspv::StorageClassCallableDataNV;
    case Pglslang::EvqCallableDataInNV: return Pspv::StorageClassIncomingCallableDataNV;
#endif
    default:
        assert(0);
        break;
    }

    return Pspv::StorageClassFunction;
}

// Add capabilities pertaining to how an array is indexed.
void TGlslangToSpvTraverser::addIndirectionIndexCapabilities(const Pglslang::TType& baseType,
                                                             const Pglslang::TType& indexType)
{
#ifndef GLSLANG_WEB
    if (indexType.getQualifier().isNonUniform()) {
        // deal with an asserted non-uniform index
        // SPV_EXT_descriptor_indexing already added in TranslateNonUniformDecoration
        if (baseType.getBasicType() == Pglslang::EbtSampler) {
            if (baseType.getQualifier().hasAttachment())
                builder.addCapability(Pspv::CapabilityInputAttachmentArrayNonUniformIndexingEXT);
            else if (baseType.isImage() && baseType.getSampler().isBuffer())
                builder.addCapability(Pspv::CapabilityStorageTexelBufferArrayNonUniformIndexingEXT);
            else if (baseType.isTexture() && baseType.getSampler().isBuffer())
                builder.addCapability(Pspv::CapabilityUniformTexelBufferArrayNonUniformIndexingEXT);
            else if (baseType.isImage())
                builder.addCapability(Pspv::CapabilityStorageImageArrayNonUniformIndexingEXT);
            else if (baseType.isTexture())
                builder.addCapability(Pspv::CapabilitySampledImageArrayNonUniformIndexingEXT);
        } else if (baseType.getBasicType() == Pglslang::EbtBlock) {
            if (baseType.getQualifier().storage == Pglslang::EvqBuffer)
                builder.addCapability(Pspv::CapabilityStorageBufferArrayNonUniformIndexingEXT);
            else if (baseType.getQualifier().storage == Pglslang::EvqUniform)
                builder.addCapability(Pspv::CapabilityUniformBufferArrayNonUniformIndexingEXT);
        }
    } else {
        // assume a dynamically uniform index
        if (baseType.getBasicType() == Pglslang::EbtSampler) {
            if (baseType.getQualifier().hasAttachment()) {
                builder.addIncorporatedExtension("SPV_EXT_descriptor_indexing", Pspv::Spv_1_5);
                builder.addCapability(Pspv::CapabilityInputAttachmentArrayDynamicIndexingEXT);
            } else if (baseType.isImage() && baseType.getSampler().isBuffer()) {
                builder.addIncorporatedExtension("SPV_EXT_descriptor_indexing", Pspv::Spv_1_5);
                builder.addCapability(Pspv::CapabilityStorageTexelBufferArrayDynamicIndexingEXT);
            } else if (baseType.isTexture() && baseType.getSampler().isBuffer()) {
                builder.addIncorporatedExtension("SPV_EXT_descriptor_indexing", Pspv::Spv_1_5);
                builder.addCapability(Pspv::CapabilityUniformTexelBufferArrayDynamicIndexingEXT);
            }
        }
    }
#endif
}

// Return whether or not the given type is something that should be tied to a
// descriptor set.
bool IsDescriptorResource(const Pglslang::TType& type)
{
    // uniform and buffer blocks are included, unless it is a push_constant
    if (type.getBasicType() == Pglslang::EbtBlock)
        return type.getQualifier().isUniformOrBuffer() &&
        ! type.getQualifier().isShaderRecordNV() &&
        ! type.getQualifier().isPushConstant();

    // non block...
    // basically samplerXXX/subpass/sampler/texture are all included
    // if they are the global-scope-class, not the function parameter
    // (or local, if they ever exist) class.
    if (type.getBasicType() == Pglslang::EbtSampler)
        return type.getQualifier().isUniformOrBuffer();

    // None of the above.
    return false;
}

void InheritQualifiers(Pglslang::TQualifier& child, const Pglslang::TQualifier& parent)
{
    if (child.layoutMatrix == Pglslang::ElmNone)
        child.layoutMatrix = parent.layoutMatrix;

    if (parent.invariant)
        child.invariant = true;
    if (parent.flat)
        child.flat = true;
    if (parent.centroid)
        child.centroid = true;
#ifndef GLSLANG_WEB
    if (parent.nopersp)
        child.nopersp = true;
    if (parent.explicitInterp)
        child.explicitInterp = true;
    if (parent.perPrimitiveNV)
        child.perPrimitiveNV = true;
    if (parent.perViewNV)
        child.perViewNV = true;
    if (parent.perTaskNV)
        child.perTaskNV = true;
    if (parent.patch)
        child.patch = true;
    if (parent.sample)
        child.sample = true;
    if (parent.coherent)
        child.coherent = true;
    if (parent.devicecoherent)
        child.devicecoherent = true;
    if (parent.queuefamilycoherent)
        child.queuefamilycoherent = true;
    if (parent.workgroupcoherent)
        child.workgroupcoherent = true;
    if (parent.subgroupcoherent)
        child.subgroupcoherent = true;
    if (parent.nonprivate)
        child.nonprivate = true;
    if (parent.volatil)
        child.volatil = true;
    if (parent.restrict)
        child.restrict = true;
    if (parent.readonly)
        child.readonly = true;
    if (parent.writeonly)
        child.writeonly = true;
#endif
}

bool HasNonLayoutQualifiers(const Pglslang::TType& type, const Pglslang::TQualifier& qualifier)
{
    // This should list qualifiers that simultaneous satisfy:
    // - struct members might inherit from a struct declaration
    //     (note that non-block structs don't explicitly inherit,
    //      only implicitly, meaning no decoration involved)
    // - affect decorations on the struct members
    //     (note smooth does not, and expecting something like volatile
    //      to effect the whole object)
    // - are not part of the offset/st430/etc or row/column-major layout
    return qualifier.invariant || (qualifier.hasLocation() && type.getBasicType() == Pglslang::EbtBlock);
}

//
// Implement the TGlslangToSpvTraverser class.
//

TGlslangToSpvTraverser::TGlslangToSpvTraverser(unsigned int spvVersion, const Pglslang::TIntermediate* glslangIntermediate,
                                               Pspv::SpvBuildLogger* buildLogger, Pglslang::SpvOptions& options)
    : TIntermTraverser(true, false, true),
      options(options),
      shaderEntry(nullptr), currentFunction(nullptr),
      sequenceDepth(0), logger(buildLogger),
      builder(spvVersion, (Pglslang::GetKhronosToolId() << 16) | Pglslang::GetSpirvGeneratorVersion(), logger),
      inEntryPoint(false), entryPointTerminated(false), linkageOnly(false),
      glslangIntermediate(glslangIntermediate),
      nanMinMaxClamp(glslangIntermediate->getNanMinMaxClamp())
{
    Pspv::ExecutionModel executionModel = TranslateExecutionModel(glslangIntermediate->getStage());

    builder.clearAccessChain();
    builder.setSource(TranslateSourceLanguage(glslangIntermediate->getSource(), glslangIntermediate->getProfile()),
                      glslangIntermediate->getVersion());

    if (options.generateDebugInfo) {
        builder.setEmitOpLines();
        builder.setSourceFile(glslangIntermediate->getSourceFile());

        // Set the source shader's text. If for SPV version 1.0, include
        // a preamble in comments stating the OpModuleProcessed instructions.
        // Otherwise, emit those as actual instructions.
        std::string text;
        const std::vector<std::string>& processes = glslangIntermediate->getProcesses();
        for (int p = 0; p < (int)processes.size(); ++p) {
            if (glslangIntermediate->getSpv().spv < Pglslang::EShTargetSpv_1_1) {
                text.append("// OpModuleProcessed ");
                text.append(processes[p]);
                text.append("\n");
            } else
                builder.addModuleProcessed(processes[p]);
        }
        if (glslangIntermediate->getSpv().spv < Pglslang::EShTargetSpv_1_1 && (int)processes.size() > 0)
            text.append("#line 1\n");
        text.append(glslangIntermediate->getSourceText());
        builder.setSourceText(text);
        // Pass name and text for all included files
        const std::map<std::string, std::string>& include_txt = glslangIntermediate->getIncludeText();
        for (auto iItr = include_txt.begin(); iItr != include_txt.end(); ++iItr)
            builder.addInclude(iItr->first, iItr->second);
    }
    stdBuiltins = builder.import("GLSL.std.450");

    Pspv::AddressingModel addressingModel = Pspv::AddressingModelLogical;
    Pspv::MemoryModel memoryModel = Pspv::MemoryModelGLSL450;

    if (glslangIntermediate->usingPhysicalStorageBuffer()) {
        addressingModel = Pspv::AddressingModelPhysicalStorageBuffer64EXT;
        builder.addIncorporatedExtension(Pspv::E_SPV_EXT_physical_storage_buffer, Pspv::Spv_1_5);
        builder.addCapability(Pspv::CapabilityPhysicalStorageBufferAddressesEXT);
    };
    if (glslangIntermediate->usingVulkanMemoryModel()) {
        memoryModel = Pspv::MemoryModelVulkanKHR;
        builder.addCapability(Pspv::CapabilityVulkanMemoryModelKHR);
        builder.addIncorporatedExtension(Pspv::E_SPV_KHR_vulkan_memory_model, Pspv::Spv_1_5);
    }
    builder.setMemoryModel(addressingModel, memoryModel);

    if (glslangIntermediate->usingVariablePointers()) {
        builder.addCapability(Pspv::CapabilityVariablePointers);
    }

    shaderEntry = builder.makeEntryPoint(glslangIntermediate->getEntryPointName().c_str());
    entryPoint = builder.addEntryPoint(executionModel, shaderEntry, glslangIntermediate->getEntryPointName().c_str());

    // Add the source extensions
    const auto& sourceExtensions = glslangIntermediate->getRequestedExtensions();
    for (auto it = sourceExtensions.begin(); it != sourceExtensions.end(); ++it)
        builder.addSourceExtension(it->c_str());

    // Add the top-level modes for this shader.

    if (glslangIntermediate->getXfbMode()) {
        builder.addCapability(Pspv::CapabilityTransformFeedback);
        builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeXfb);
    }

    unsigned int mode;
    switch (glslangIntermediate->getStage()) {
    case EShLangVertex:
        builder.addCapability(Pspv::CapabilityShader);
        break;

    case EShLangFragment:
        builder.addCapability(Pspv::CapabilityShader);
        if (glslangIntermediate->getPixelCenterInteger())
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModePixelCenterInteger);

        if (glslangIntermediate->getOriginUpperLeft())
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeOriginUpperLeft);
        else
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeOriginLowerLeft);

        if (glslangIntermediate->getEarlyFragmentTests())
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeEarlyFragmentTests);

        if (glslangIntermediate->getPostDepthCoverage()) {
            builder.addCapability(Pspv::CapabilitySampleMaskPostDepthCoverage);
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModePostDepthCoverage);
            builder.addExtension(Pspv::E_SPV_KHR_post_depth_coverage);
        }

        if (glslangIntermediate->getDepth() != Pglslang::EldUnchanged && glslangIntermediate->isDepthReplacing())
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeDepthReplacing);

#ifndef GLSLANG_WEB
        switch(glslangIntermediate->getDepth()) {
        case Pglslang::EldGreater:  mode = Pspv::ExecutionModeDepthGreater; break;
        case Pglslang::EldLess:     mode = Pspv::ExecutionModeDepthLess;    break;
        default:                   mode = Pspv::ExecutionModeMax;          break;
        }
        if (mode != Pspv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (Pspv::ExecutionMode)mode);
        switch (glslangIntermediate->getInterlockOrdering()) {
        case Pglslang::EioPixelInterlockOrdered:         mode = Pspv::ExecutionModePixelInterlockOrderedEXT;
            break;
        case Pglslang::EioPixelInterlockUnordered:       mode = Pspv::ExecutionModePixelInterlockUnorderedEXT;
            break;
        case Pglslang::EioSampleInterlockOrdered:        mode = Pspv::ExecutionModeSampleInterlockOrderedEXT;
            break;
        case Pglslang::EioSampleInterlockUnordered:      mode = Pspv::ExecutionModeSampleInterlockUnorderedEXT;
            break;
        case Pglslang::EioShadingRateInterlockOrdered:   mode = Pspv::ExecutionModeShadingRateInterlockOrderedEXT;
            break;
        case Pglslang::EioShadingRateInterlockUnordered: mode = Pspv::ExecutionModeShadingRateInterlockUnorderedEXT;
            break;
        default:                                        mode = Pspv::ExecutionModeMax;
            break;
        }
        if (mode != Pspv::ExecutionModeMax) {
            builder.addExecutionMode(shaderEntry, (Pspv::ExecutionMode)mode);
            if (mode == Pspv::ExecutionModeShadingRateInterlockOrderedEXT ||
                mode == Pspv::ExecutionModeShadingRateInterlockUnorderedEXT) {
                builder.addCapability(Pspv::CapabilityFragmentShaderShadingRateInterlockEXT);
            } else if (mode == Pspv::ExecutionModePixelInterlockOrderedEXT ||
                       mode == Pspv::ExecutionModePixelInterlockUnorderedEXT) {
                builder.addCapability(Pspv::CapabilityFragmentShaderPixelInterlockEXT);
            } else {
                builder.addCapability(Pspv::CapabilityFragmentShaderSampleInterlockEXT);
            }
            builder.addExtension(Pspv::E_SPV_EXT_fragment_shader_interlock);
        }
#endif
        break;

    case EShLangCompute:
        builder.addCapability(Pspv::CapabilityShader);
        builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeLocalSize, glslangIntermediate->getLocalSize(0),
                                                                           glslangIntermediate->getLocalSize(1),
                                                                           glslangIntermediate->getLocalSize(2));
        if (glslangIntermediate->getLayoutDerivativeModeNone() == Pglslang::LayoutDerivativeGroupQuads) {
            builder.addCapability(Pspv::CapabilityComputeDerivativeGroupQuadsNV);
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeDerivativeGroupQuadsNV);
            builder.addExtension(Pspv::E_SPV_NV_compute_shader_derivatives);
        } else if (glslangIntermediate->getLayoutDerivativeModeNone() == Pglslang::LayoutDerivativeGroupLinear) {
            builder.addCapability(Pspv::CapabilityComputeDerivativeGroupLinearNV);
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeDerivativeGroupLinearNV);
            builder.addExtension(Pspv::E_SPV_NV_compute_shader_derivatives);
        }
        break;
#ifndef GLSLANG_WEB
    case EShLangTessEvaluation:
    case EShLangTessControl:
        builder.addCapability(Pspv::CapabilityTessellation);

        Pglslang::TLayoutGeometry primitive;

        if (glslangIntermediate->getStage() == EShLangTessControl) {
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeOutputVertices, glslangIntermediate->getVertices());
            primitive = glslangIntermediate->getOutputPrimitive();
        } else {
            primitive = glslangIntermediate->getInputPrimitive();
        }

        switch (primitive) {
        case Pglslang::ElgTriangles:           mode = Pspv::ExecutionModeTriangles;     break;
        case Pglslang::ElgQuads:               mode = Pspv::ExecutionModeQuads;         break;
        case Pglslang::ElgIsolines:            mode = Pspv::ExecutionModeIsolines;      break;
        default:                              mode = Pspv::ExecutionModeMax;           break;
        }
        if (mode != Pspv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (Pspv::ExecutionMode)mode);

        switch (glslangIntermediate->getVertexSpacing()) {
        case Pglslang::EvsEqual:            mode = Pspv::ExecutionModeSpacingEqual;          break;
        case Pglslang::EvsFractionalEven:   mode = Pspv::ExecutionModeSpacingFractionalEven; break;
        case Pglslang::EvsFractionalOdd:    mode = Pspv::ExecutionModeSpacingFractionalOdd;  break;
        default:                           mode = Pspv::ExecutionModeMax;                   break;
        }
        if (mode != Pspv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (Pspv::ExecutionMode)mode);

        switch (glslangIntermediate->getVertexOrder()) {
        case Pglslang::EvoCw:     mode = Pspv::ExecutionModeVertexOrderCw;  break;
        case Pglslang::EvoCcw:    mode = Pspv::ExecutionModeVertexOrderCcw; break;
        default:                 mode = Pspv::ExecutionModeMax;            break;
        }
        if (mode != Pspv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (Pspv::ExecutionMode)mode);

        if (glslangIntermediate->getPointMode())
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModePointMode);
        break;

    case EShLangGeometry:
        builder.addCapability(Pspv::CapabilityGeometry);
        switch (glslangIntermediate->getInputPrimitive()) {
        case Pglslang::ElgPoints:             mode = Pspv::ExecutionModeInputPoints;             break;
        case Pglslang::ElgLines:              mode = Pspv::ExecutionModeInputLines;              break;
        case Pglslang::ElgLinesAdjacency:     mode = Pspv::ExecutionModeInputLinesAdjacency;     break;
        case Pglslang::ElgTriangles:          mode = Pspv::ExecutionModeTriangles;               break;
        case Pglslang::ElgTrianglesAdjacency: mode = Pspv::ExecutionModeInputTrianglesAdjacency; break;
        default:                             mode = Pspv::ExecutionModeMax;                     break;
        }
        if (mode != Pspv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (Pspv::ExecutionMode)mode);

        builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeInvocations, glslangIntermediate->getInvocations());

        switch (glslangIntermediate->getOutputPrimitive()) {
        case Pglslang::ElgPoints:        mode = Pspv::ExecutionModeOutputPoints;                 break;
        case Pglslang::ElgLineStrip:     mode = Pspv::ExecutionModeOutputLineStrip;              break;
        case Pglslang::ElgTriangleStrip: mode = Pspv::ExecutionModeOutputTriangleStrip;          break;
        default:                        mode = Pspv::ExecutionModeMax;                          break;
        }
        if (mode != Pspv::ExecutionModeMax)
            builder.addExecutionMode(shaderEntry, (Pspv::ExecutionMode)mode);
        builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeOutputVertices, glslangIntermediate->getVertices());
        break;

    case EShLangRayGenNV:
    case EShLangIntersectNV:
    case EShLangAnyHitNV:
    case EShLangClosestHitNV:
    case EShLangMissNV:
    case EShLangCallableNV:
        builder.addCapability(Pspv::CapabilityRayTracingNV);
        builder.addExtension("SPV_NV_ray_tracing");
        break;
    case EShLangTaskNV:
    case EShLangMeshNV:
        builder.addCapability(Pspv::CapabilityMeshShadingNV);
        builder.addExtension(Pspv::E_SPV_NV_mesh_shader);
        builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeLocalSize, glslangIntermediate->getLocalSize(0),
                                                                           glslangIntermediate->getLocalSize(1),
                                                                           glslangIntermediate->getLocalSize(2));
        if (glslangIntermediate->getStage() == EShLangMeshNV) {
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeOutputVertices, glslangIntermediate->getVertices());
            builder.addExecutionMode(shaderEntry, Pspv::ExecutionModeOutputPrimitivesNV, glslangIntermediate->getPrimitives());

            switch (glslangIntermediate->getOutputPrimitive()) {
            case Pglslang::ElgPoints:        mode = Pspv::ExecutionModeOutputPoints;      break;
            case Pglslang::ElgLines:         mode = Pspv::ExecutionModeOutputLinesNV;     break;
            case Pglslang::ElgTriangles:     mode = Pspv::ExecutionModeOutputTrianglesNV; break;
            default:                        mode = Pspv::ExecutionModeMax;               break;
            }
            if (mode != Pspv::ExecutionModeMax)
                builder.addExecutionMode(shaderEntry, (Pspv::ExecutionMode)mode);
        }
        break;
#endif

    default:
        break;
    }
}

// Finish creating SPV, after the traversal is complete.
void TGlslangToSpvTraverser::finishSpv()
{
    // Finish the entry point function
    if (! entryPointTerminated) {
        builder.setBuildPoint(shaderEntry->getLastBlock());
        builder.leaveFunction();
    }

    // finish off the entry-point SPV instruction by adding the Input/Output <id>
    for (auto it = iOSet.cbegin(); it != iOSet.cend(); ++it)
        entryPoint->addIdOperand(*it);

    // Add capabilities, extensions, remove unneeded decorations, etc.,
    // based on the resulting SPIR-V.
    // Note: WebGPU code generation must have the opportunity to aggressively
    // prune unreachable merge blocks and continue targets.
    builder.postProcess();
}

// Write the SPV into 'out'.
void TGlslangToSpvTraverser::dumpSpv(std::vector<unsigned int>& out)
{
    builder.dump(out);
}

//
// Implement the traversal functions.
//
// Return true from interior nodes to have the external traversal
// continue on to children.  Return false if children were
// already processed.
//

//
// Symbols can turn into
//  - uniform/input reads
//  - output writes
//  - complex lvalue base setups:  foo.bar[3]....  , where we see foo and start up an access chain
//  - something simple that degenerates into the last bullet
//
void TGlslangToSpvTraverser::visitSymbol(Pglslang::TIntermSymbol* symbol)
{
    SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
    if (symbol->getType().isStruct())
        glslangTypeToIdMap[symbol->getType().getStruct()] = symbol->getId();

    if (symbol->getType().getQualifier().isSpecConstant())
        spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

    // getSymbolId() will set up all the IO decorations on the first call.
    // Formal function parameters were mapped during makeFunctions().
    Pspv::Id id = getSymbolId(symbol);

    if (builder.isPointer(id)) {
        // Include all "static use" and "linkage only" interface variables on the OpEntryPoint instruction
        // Consider adding to the OpEntryPoint interface list.
        // Only looking at structures if they have at least one member.
        if (!symbol->getType().isStruct() || symbol->getType().getStruct()->size() > 0) {
            Pspv::StorageClass sc = builder.getStorageClass(id);
            // Before SPIR-V 1.4, we only want to include Input and Output.
            // Starting with SPIR-V 1.4, we want all globals.
            if ((glslangIntermediate->getSpv().spv >= Pglslang::EShTargetSpv_1_4 && sc != Pspv::StorageClassFunction) ||
                (sc == Pspv::StorageClassInput || sc == Pspv::StorageClassOutput)) {
                iOSet.insert(id);
            }
        }

        // If the SPIR-V type is required to be different than the AST type,
        // translate now from the SPIR-V type to the AST type, for the consuming
        // operation.
        // Note this turns it from an l-value to an r-value.
        // Currently, all symbols needing this are inputs; avoid the map lookup when non-input.
        if (symbol->getType().getQualifier().storage == Pglslang::EvqVaryingIn)
            id = translateForcedType(id);
    }

    // Only process non-linkage-only nodes for generating actual static uses
    if (! linkageOnly || symbol->getQualifier().isSpecConstant()) {
        // Prepare to generate code for the access

        // L-value chains will be computed left to right.  We're on the symbol now,
        // which is the left-most part of the access chain, so now is "clear" time,
        // followed by setting the base.
        builder.clearAccessChain();

        // For now, we consider all user variables as being in memory, so they are pointers,
        // except for
        // A) R-Value arguments to a function, which are an intermediate object.
        //    See comments in handleUserFunctionCall().
        // B) Specialization constants (normal constants don't even come in as a variable),
        //    These are also pure R-values.
        // C) R-Values from type translation, see above call to translateForcedType()
        Pglslang::TQualifier qualifier = symbol->getQualifier();
        if (qualifier.isSpecConstant() || rValueParameters.find(symbol->getId()) != rValueParameters.end() ||
            !builder.isPointerType(builder.getTypeId(id)))
            builder.setAccessChainRValue(id);
        else
            builder.setAccessChainLValue(id);
    }

#ifdef ENABLE_HLSL
    // Process linkage-only nodes for any special additional interface work.
    if (linkageOnly) {
        if (glslangIntermediate->getHlslFunctionality1()) {
            // Map implicit counter buffers to their originating buffers, which should have been
            // seen by now, given earlier pruning of unused counters, and preservation of order
            // of declaration.
            if (symbol->getType().getQualifier().isUniformOrBuffer()) {
                if (!glslangIntermediate->hasCounterBufferName(symbol->getName())) {
                    // Save possible originating buffers for counter buffers, keyed by
                    // making the potential counter-buffer name.
                    std::string keyName = symbol->getName().c_str();
                    keyName = glslangIntermediate->addCounterBufferName(keyName);
                    counterOriginator[keyName] = symbol;
                } else {
                    // Handle a counter buffer, by finding the saved originating buffer.
                    std::string keyName = symbol->getName().c_str();
                    auto it = counterOriginator.find(keyName);
                    if (it != counterOriginator.end()) {
                        id = getSymbolId(it->second);
                        if (id != Pspv::NoResult) {
                            Pspv::Id counterId = getSymbolId(symbol);
                            if (counterId != Pspv::NoResult) {
                                builder.addExtension("SPV_GOOGLE_hlsl_functionality1");
                                builder.addDecorationId(id, Pspv::DecorationHlslCounterBufferGOOGLE, counterId);
                            }
                        }
                    }
                }
            }
        }
    }
#endif
}

bool TGlslangToSpvTraverser::visitBinary(Pglslang::TVisit /* visit */, Pglslang::TIntermBinary* node)
{
    builder.setLine(node->getLoc().line, node->getLoc().getFilename());
    if (node->getLeft()->getAsSymbolNode() != nullptr && node->getLeft()->getType().isStruct()) {
        glslangTypeToIdMap[node->getLeft()->getType().getStruct()] = node->getLeft()->getAsSymbolNode()->getId();
    }
    if (node->getRight()->getAsSymbolNode() != nullptr && node->getRight()->getType().isStruct()) {
        glslangTypeToIdMap[node->getRight()->getType().getStruct()] = node->getRight()->getAsSymbolNode()->getId();
    }

    SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
    if (node->getType().getQualifier().isSpecConstant())
        spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

    // First, handle special cases
    switch (node->getOp()) {
    case Pglslang::EOpAssign:
    case Pglslang::EOpAddAssign:
    case Pglslang::EOpSubAssign:
    case Pglslang::EOpMulAssign:
    case Pglslang::EOpVectorTimesMatrixAssign:
    case Pglslang::EOpVectorTimesScalarAssign:
    case Pglslang::EOpMatrixTimesScalarAssign:
    case Pglslang::EOpMatrixTimesMatrixAssign:
    case Pglslang::EOpDivAssign:
    case Pglslang::EOpModAssign:
    case Pglslang::EOpAndAssign:
    case Pglslang::EOpInclusiveOrAssign:
    case Pglslang::EOpExclusiveOrAssign:
    case Pglslang::EOpLeftShiftAssign:
    case Pglslang::EOpRightShiftAssign:
        // A bin-op assign "a += b" means the same thing as "a = a + b"
        // where a is evaluated before b. For a simple assignment, GLSL
        // says to evaluate the left before the right.  So, always, left
        // node then right node.
        {
            // get the left l-value, save it away
            builder.clearAccessChain();
            node->getLeft()->traverse(this);
            Pspv::Builder::AccessChain lValue = builder.getAccessChain();

            // evaluate the right
            builder.clearAccessChain();
            node->getRight()->traverse(this);
            Pspv::Id rValue = accessChainLoad(node->getRight()->getType());

            if (node->getOp() != Pglslang::EOpAssign) {
                // the left is also an r-value
                builder.setAccessChain(lValue);
                Pspv::Id leftRValue = accessChainLoad(node->getLeft()->getType());

                // do the operation
                OpDecorations decorations = { TranslatePrecisionDecoration(node->getOperationPrecision()),
                                              TranslateNoContractionDecoration(node->getType().getQualifier()),
                                              TranslateNonUniformDecoration(node->getType().getQualifier()) };
                rValue = createBinaryOperation(node->getOp(), decorations,
                                               convertGlslangToSpvType(node->getType()), leftRValue, rValue,
                                               node->getType().getBasicType());

                // these all need their counterparts in createBinaryOperation()
                assert(rValue != Pspv::NoResult);
            }

            // store the result
            builder.setAccessChain(lValue);
            multiTypeStore(node->getLeft()->getType(), rValue);

            // assignments are expressions having an rValue after they are evaluated...
            builder.clearAccessChain();
            builder.setAccessChainRValue(rValue);
        }
        return false;
    case Pglslang::EOpIndexDirect:
    case Pglslang::EOpIndexDirectStruct:
        {
            // Structure, array, matrix, or vector indirection with statically known index.
            // Get the left part of the access chain.
            node->getLeft()->traverse(this);

            // Add the next element in the chain

            const int glslangIndex = node->getRight()->getAsConstantUnion()->getConstArray()[0].getIConst();
            if (! node->getLeft()->getType().isArray() &&
                node->getLeft()->getType().isVector() &&
                node->getOp() == Pglslang::EOpIndexDirect) {
                // This is essentially a hard-coded vector swizzle of size 1,
                // so short circuit the access-chain stuff with a swizzle.
                std::vector<unsigned> swizzle;
                swizzle.push_back(glslangIndex);
                int dummySize;
                builder.accessChainPushSwizzle(swizzle, convertGlslangToSpvType(node->getLeft()->getType()),
                                               TranslateCoherent(node->getLeft()->getType()),
                                               glslangIntermediate->PgetBaseAlignmentScalar(node->getLeft()->getType(), dummySize));
            } else {

                // Load through a block reference is performed with a dot operator that
                // is mapped to EOpIndexDirectStruct. When we get to the actual reference,
                // do a load and reset the access chain.
                if (node->getLeft()->isReference() &&
                    !node->getLeft()->getType().isArray() &&
                    node->getOp() == Pglslang::EOpIndexDirectStruct)
                {
                    Pspv::Id left = accessChainLoad(node->getLeft()->getType());
                    builder.clearAccessChain();
                    builder.setAccessChainLValue(left);
                }

                int spvIndex = glslangIndex;
                if (node->getLeft()->getBasicType() == Pglslang::EbtBlock &&
                    node->getOp() == Pglslang::EOpIndexDirectStruct)
                {
                    // This may be, e.g., an anonymous block-member selection, which generally need
                    // index remapping due to hidden members in anonymous blocks.
                    int glslangId = glslangTypeToIdMap[node->getLeft()->getType().getStruct()];
                    if (memberRemapper.find(glslangId) != memberRemapper.end()) {
                        std::vector<int>& remapper = memberRemapper[glslangId];
                        assert(remapper.size() > 0);
                        spvIndex = remapper[glslangIndex];
                    }
                }

                // normal case for indexing array or structure or block
                builder.accessChainPush(builder.makeIntConstant(spvIndex), TranslateCoherent(node->getLeft()->getType()), node->getLeft()->getType().getBufferReferenceAlignment());

                // Add capabilities here for accessing PointSize and clip/cull distance.
                // We have deferred generation of associated capabilities until now.
                if (node->getLeft()->getType().isStruct() && ! node->getLeft()->getType().isArray())
                    declareUseOfStructMember(*(node->getLeft()->getType().getStruct()), glslangIndex);
            }
        }
        return false;
    case Pglslang::EOpIndexIndirect:
        {
            // Array, matrix, or vector indirection with variable index.
            // Will use native SPIR-V access-chain for and array indirection;
            // matrices are arrays of vectors, so will also work for a matrix.
            // Will use the access chain's 'component' for variable index into a vector.

            // This adapter is building access chains left to right.
            // Set up the access chain to the left.
            node->getLeft()->traverse(this);

            // save it so that computing the right side doesn't trash it
            Pspv::Builder::AccessChain partial = builder.getAccessChain();

            // compute the next index in the chain
            builder.clearAccessChain();
            node->getRight()->traverse(this);
            Pspv::Id index = accessChainLoad(node->getRight()->getType());

            addIndirectionIndexCapabilities(node->getLeft()->getType(), node->getRight()->getType());

            // restore the saved access chain
            builder.setAccessChain(partial);

            if (! node->getLeft()->getType().isArray() && node->getLeft()->getType().isVector()) {
                int dummySize;
                builder.accessChainPushComponent(index, convertGlslangToSpvType(node->getLeft()->getType()),
                                                TranslateCoherent(node->getLeft()->getType()),
                                                glslangIntermediate->PgetBaseAlignmentScalar(node->getLeft()->getType(), dummySize));
            } else
                builder.accessChainPush(index, TranslateCoherent(node->getLeft()->getType()), node->getLeft()->getType().getBufferReferenceAlignment());
        }
        return false;
    case Pglslang::EOpVectorSwizzle:
        {
            node->getLeft()->traverse(this);
            std::vector<unsigned> swizzle;
            convertSwizzle(*node->getRight()->getAsAggregate(), swizzle);
            int dummySize;
            builder.accessChainPushSwizzle(swizzle, convertGlslangToSpvType(node->getLeft()->getType()),
                                           TranslateCoherent(node->getLeft()->getType()),
                                           glslangIntermediate->PgetBaseAlignmentScalar(node->getLeft()->getType(), dummySize));
        }
        return false;
    case Pglslang::EOpMatrixSwizzle:
        logger->missingFunctionality("matrix swizzle");
        return true;
    case Pglslang::EOpLogicalOr:
    case Pglslang::EOpLogicalAnd:
        {

            // These may require short circuiting, but can sometimes be done as straight
            // binary operations.  The right operand must be short circuited if it has
            // side effects, and should probably be if it is complex.
            if (isTrivial(node->getRight()->getAsTyped()))
                break; // handle below as a normal binary operation
            // otherwise, we need to do dynamic short circuiting on the right operand
            Pspv::Id result = createShortCircuit(node->getOp(), *node->getLeft()->getAsTyped(), *node->getRight()->getAsTyped());
            builder.clearAccessChain();
            builder.setAccessChainRValue(result);
        }
        return false;
    default:
        break;
    }

    // Assume generic binary op...

    // get right operand
    builder.clearAccessChain();
    node->getLeft()->traverse(this);
    Pspv::Id left = accessChainLoad(node->getLeft()->getType());

    // get left operand
    builder.clearAccessChain();
    node->getRight()->traverse(this);
    Pspv::Id right = accessChainLoad(node->getRight()->getType());

    // get result
    OpDecorations decorations = { TranslatePrecisionDecoration(node->getOperationPrecision()),
                                  TranslateNoContractionDecoration(node->getType().getQualifier()),
                                  TranslateNonUniformDecoration(node->getType().getQualifier()) };
    Pspv::Id result = createBinaryOperation(node->getOp(), decorations,
                                           convertGlslangToSpvType(node->getType()), left, right,
                                           node->getLeft()->getType().getBasicType());

    builder.clearAccessChain();
    if (! result) {
        logger->missingFunctionality("unknown glslang binary operation");
        return true;  // pick up a child as the place-holder result
    } else {
        builder.setAccessChainRValue(result);
        return false;
    }
}

// Figure out what, if any, type changes are needed when accessing a specific built-in.
// Returns <the type SPIR-V requires for declarion, the type to translate to on use>.
// Also see comment for 'forceType', regarding tracking SPIR-V-required types.
std::pair<Pspv::Id, Pspv::Id> TGlslangToSpvTraverser::getForcedType(Pspv::BuiltIn builtIn,
    const Pglslang::TType& glslangType)
{
    switch(builtIn)
    {
        case Pspv::BuiltInSubgroupEqMask:
        case Pspv::BuiltInSubgroupGeMask:
        case Pspv::BuiltInSubgroupGtMask:
        case Pspv::BuiltInSubgroupLeMask:
        case Pspv::BuiltInSubgroupLtMask: {
            // these require changing a 64-bit scaler -> a vector of 32-bit components
            if (glslangType.isVector())
                break;
            std::pair<Pspv::Id, Pspv::Id> ret(builder.makeVectorType(builder.makeUintType(32), 4),
                                            builder.makeUintType(64));
            return ret;
        }
        default:
            break;
    }

    std::pair<Pspv::Id, Pspv::Id> ret(Pspv::NoType, Pspv::NoType);
    return ret;
}

// For an object previously identified (see getForcedType() and forceType)
// as needing type translations, do the translation needed for a load, turning
// an L-value into in R-value.
Pspv::Id TGlslangToSpvTraverser::translateForcedType(Pspv::Id object)
{
    const auto forceIt = forceType.find(object);
    if (forceIt == forceType.end())
        return object;

    Pspv::Id desiredTypeId = forceIt->second;
    Pspv::Id objectTypeId = builder.getTypeId(object);
    assert(builder.isPointerType(objectTypeId));
    objectTypeId = builder.getContainedTypeId(objectTypeId);
    if (builder.isVectorType(objectTypeId) &&
        builder.getScalarTypeWidth(builder.getContainedTypeId(objectTypeId)) == 32) {
        if (builder.getScalarTypeWidth(desiredTypeId) == 64) {
            // handle 32-bit v.xy* -> 64-bit
            builder.clearAccessChain();
            builder.setAccessChainLValue(object);
            object = builder.accessChainLoad(Pspv::NoPrecision, Pspv::DecorationMax, objectTypeId);
            std::vector<Pspv::Id> components;
            components.push_back(builder.createCompositeExtract(object, builder.getContainedTypeId(objectTypeId), 0));
            components.push_back(builder.createCompositeExtract(object, builder.getContainedTypeId(objectTypeId), 1));

            Pspv::Id vecType = builder.makeVectorType(builder.getContainedTypeId(objectTypeId), 2);
            return builder.createUnaryOp(Pspv::OpBitcast, desiredTypeId,
                                         builder.createCompositeConstruct(vecType, components));
        } else {
            logger->missingFunctionality("forcing 32-bit vector type to non 64-bit scalar");
        }
    } else {
        logger->missingFunctionality("forcing non 32-bit vector type");
    }

    return object;
}

bool TGlslangToSpvTraverser::visitUnary(Pglslang::TVisit /* visit */, Pglslang::TIntermUnary* node)
{
    builder.setLine(node->getLoc().line, node->getLoc().getFilename());

    SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
    if (node->getType().getQualifier().isSpecConstant())
        spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

    Pspv::Id result = Pspv::NoResult;

    // try texturing first
    result = createImageTextureFunctionCall(node);
    if (result != Pspv::NoResult) {
        builder.clearAccessChain();
        builder.setAccessChainRValue(result);

        return false; // done with this node
    }

    // Non-texturing.

    if (node->getOp() == Pglslang::EOpArrayLength) {
        // Quite special; won't want to evaluate the operand.

        // Currently, the front-end does not allow .length() on an array until it is sized,
        // except for the last block membeor of an SSBO.
        // TODO: If this changes, link-time sized arrays might show up here, and need their
        // size extracted.

        // Normal .length() would have been constant folded by the front-end.
        // So, this has to be block.lastMember.length().
        // SPV wants "block" and member number as the operands, go get them.

        Pspv::Id length;
        if (node->getOperand()->getType().isCoopMat()) {
            spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

            Pspv::Id typeId = convertGlslangToSpvType(node->getOperand()->getType());
            assert(builder.isCooperativeMatrixType(typeId));

            length = builder.createCooperativeMatrixLength(typeId);
        } else {
            Pglslang::TIntermTyped* block = node->getOperand()->getAsBinaryNode()->getLeft();
            block->traverse(this);
            unsigned int member = node->getOperand()->getAsBinaryNode()->getRight()->getAsConstantUnion()->getConstArray()[0].getUConst();
            length = builder.createArrayLength(builder.accessChainGetLValue(), member);
        }

        // GLSL semantics say the result of .length() is an int, while SPIR-V says
        // signedness must be 0. So, convert from SPIR-V unsigned back to GLSL's
        // AST expectation of a signed result.
        if (glslangIntermediate->getSource() == Pglslang::EShSourceGlsl) {
            if (builder.isInSpecConstCodeGenMode()) {
                length = builder.createBinOp(Pspv::OpIAdd, builder.makeIntType(32), length, builder.makeIntConstant(0));
            } else {
                length = builder.createUnaryOp(Pspv::OpBitcast, builder.makeIntType(32), length);
            }
        }

        builder.clearAccessChain();
        builder.setAccessChainRValue(length);

        return false;
    }

    // Start by evaluating the operand

    // Does it need a swizzle inversion?  If so, evaluation is inverted;
    // operate first on the swizzle base, then apply the swizzle.
    Pspv::Id invertedType = Pspv::NoType;
    auto resultType = [&invertedType, &node, this](){ return invertedType != Pspv::NoType ? invertedType : convertGlslangToSpvType(node->getType()); };
    if (node->getOp() == Pglslang::EOpInterpolateAtCentroid)
        invertedType = getInvertedSwizzleType(*node->getOperand());

    builder.clearAccessChain();
    TIntermNode *operandNode;
    if (invertedType != Pspv::NoType)
        operandNode = node->getOperand()->getAsBinaryNode()->getLeft();
    else
        operandNode = node->getOperand();
    
    operandNode->traverse(this);

    Pspv::Id operand = Pspv::NoResult;

    Pspv::Builder::AccessChain::CoherentFlags lvalueCoherentFlags;

#ifndef GLSLANG_WEB
    if (node->getOp() == Pglslang::EOpAtomicCounterIncrement ||
        node->getOp() == Pglslang::EOpAtomicCounterDecrement ||
        node->getOp() == Pglslang::EOpAtomicCounter          ||
        node->getOp() == Pglslang::EOpInterpolateAtCentroid) {
        operand = builder.accessChainGetLValue(); // Special case l-value operands
        lvalueCoherentFlags = builder.getAccessChain().coherentFlags;
        lvalueCoherentFlags |= TranslateCoherent(operandNode->getAsTyped()->getType());
    } else
#endif
    {
        operand = accessChainLoad(node->getOperand()->getType());
    }

    OpDecorations decorations = { TranslatePrecisionDecoration(node->getOperationPrecision()),
                                  TranslateNoContractionDecoration(node->getType().getQualifier()),
                                  TranslateNonUniformDecoration(node->getType().getQualifier()) };

    // it could be a conversion
    if (! result)
        result = createConversion(node->getOp(), decorations, resultType(), operand, node->getOperand()->getBasicType());

    // if not, then possibly an operation
    if (! result)
        result = createUnaryOperation(node->getOp(), decorations, resultType(), operand, node->getOperand()->getBasicType(), lvalueCoherentFlags);

    if (result) {
        if (invertedType) {
            result = createInvertedSwizzle(decorations.precision, *node->getOperand(), result);
            decorations.addNonUniform(builder, result);
        }

        builder.clearAccessChain();
        builder.setAccessChainRValue(result);

        return false; // done with this node
    }

    // it must be a special case, check...
    switch (node->getOp()) {
    case Pglslang::EOpPostIncrement:
    case Pglslang::EOpPostDecrement:
    case Pglslang::EOpPreIncrement:
    case Pglslang::EOpPreDecrement:
        {
            // we need the integer value "1" or the floating point "1.0" to add/subtract
            Pspv::Id one = 0;
            if (node->getBasicType() == Pglslang::EbtFloat)
                one = builder.makeFloatConstant(1.0F);
#ifndef GLSLANG_WEB
            else if (node->getBasicType() == Pglslang::EbtDouble)
                one = builder.makeDoubleConstant(1.0);
            else if (node->getBasicType() == Pglslang::EbtFloat16)
                one = builder.makeFloat16Constant(1.0F);
            else if (node->getBasicType() == Pglslang::EbtInt8  || node->getBasicType() == Pglslang::EbtUint8)
                one = builder.makeInt8Constant(1);
            else if (node->getBasicType() == Pglslang::EbtInt16 || node->getBasicType() == Pglslang::EbtUint16)
                one = builder.makeInt16Constant(1);
            else if (node->getBasicType() == Pglslang::EbtInt64 || node->getBasicType() == Pglslang::EbtUint64)
                one = builder.makeInt64Constant(1);
#endif
            else
                one = builder.makeIntConstant(1);
            Pglslang::TOperator op;
            if (node->getOp() == Pglslang::EOpPreIncrement ||
                node->getOp() == Pglslang::EOpPostIncrement)
                op = Pglslang::EOpAdd;
            else
                op = Pglslang::EOpSub;

            Pspv::Id result = createBinaryOperation(op, decorations,
                                                   convertGlslangToSpvType(node->getType()), operand, one,
                                                   node->getType().getBasicType());
            assert(result != Pspv::NoResult);

            // The result of operation is always stored, but conditionally the
            // consumed result.  The consumed result is always an r-value.
            builder.accessChainStore(result);
            builder.clearAccessChain();
            if (node->getOp() == Pglslang::EOpPreIncrement ||
                node->getOp() == Pglslang::EOpPreDecrement)
                builder.setAccessChainRValue(result);
            else
                builder.setAccessChainRValue(operand);
        }

        return false;

#ifndef GLSLANG_WEB
    case Pglslang::EOpEmitStreamVertex:
        builder.createNoResultOp(Pspv::OpEmitStreamVertex, operand);
        return false;
    case Pglslang::EOpEndStreamPrimitive:
        builder.createNoResultOp(Pspv::OpEndStreamPrimitive, operand);
        return false;
#endif

    default:
        logger->missingFunctionality("unknown glslang unary");
        return true;  // pick up operand as placeholder result
    }
}

// Construct a composite object, recursively copying members if their types don't match
Pspv::Id TGlslangToSpvTraverser::createCompositeConstruct(Pspv::Id resultTypeId, std::vector<Pspv::Id> constituents)
{
    for (int c = 0; c < (int)constituents.size(); ++c) {
        Pspv::Id& constituent = constituents[c];
        Pspv::Id lType = builder.getContainedTypeId(resultTypeId, c);
        Pspv::Id rType = builder.getTypeId(constituent);
        if (lType != rType) {
            if (glslangIntermediate->getSpv().spv >= Pglslang::EShTargetSpv_1_4) {
                constituent = builder.createUnaryOp(Pspv::OpCopyLogical, lType, constituent);
            } else if (builder.isStructType(rType)) {
                std::vector<Pspv::Id> rTypeConstituents;
                int numrTypeConstituents = builder.getNumTypeConstituents(rType);
                for (int i = 0; i < numrTypeConstituents; ++i) {
                    rTypeConstituents.push_back(builder.createCompositeExtract(constituent, builder.getContainedTypeId(rType, i), i));
                }
                constituents[c] = createCompositeConstruct(lType, rTypeConstituents);
            } else {
                assert(builder.isArrayType(rType));
                std::vector<Pspv::Id> rTypeConstituents;
                int numrTypeConstituents = builder.getNumTypeConstituents(rType);

                Pspv::Id elementRType = builder.getContainedTypeId(rType);
                for (int i = 0; i < numrTypeConstituents; ++i) {
                    rTypeConstituents.push_back(builder.createCompositeExtract(constituent, elementRType, i));
                }
                constituents[c] = createCompositeConstruct(lType, rTypeConstituents);
            }
        }
    }
    return builder.createCompositeConstruct(resultTypeId, constituents);
}

bool TGlslangToSpvTraverser::visitAggregate(Pglslang::TVisit visit, Pglslang::TIntermAggregate* node)
{
    SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
    if (node->getType().getQualifier().isSpecConstant())
        spec_constant_op_mode_setter.turnOnSpecConstantOpMode();

    Pspv::Id result = Pspv::NoResult;
    Pspv::Id invertedType = Pspv::NoType;  // to use to override the natural type of the node
    auto resultType = [&invertedType, &node, this](){ return invertedType != Pspv::NoType ? invertedType : convertGlslangToSpvType(node->getType()); };

    // try texturing
    result = createImageTextureFunctionCall(node);
    if (result != Pspv::NoResult) {
        builder.clearAccessChain();
        builder.setAccessChainRValue(result);

        return false;
    }
#ifndef GLSLANG_WEB
    else if (node->getOp() == Pglslang::EOpImageStore ||
        node->getOp() == Pglslang::EOpImageStoreLod ||
        node->getOp() == Pglslang::EOpImageAtomicStore) {
        // "imageStore" is a special case, which has no result
        return false;
    }
#endif

    Pglslang::TOperator binOp = Pglslang::EOpNull;
    bool reduceComparison = true;
    bool isMatrix = false;
    bool noReturnValue = false;
    bool atomic = false;

    Pspv::Builder::AccessChain::CoherentFlags lvalueCoherentFlags;

    assert(node->getOp());

    Pspv::Decoration precision = TranslatePrecisionDecoration(node->getOperationPrecision());

    switch (node->getOp()) {
    case Pglslang::EOpSequence:
    {
        if (preVisit)
            ++sequenceDepth;
        else
            --sequenceDepth;

        if (sequenceDepth == 1) {
            // If this is the parent node of all the functions, we want to see them
            // early, so all call points have actual SPIR-V functions to reference.
            // In all cases, still let the traverser visit the children for us.
            makeFunctions(node->getAsAggregate()->getSequence());

            // Also, we want all globals initializers to go into the beginning of the entry point, before
            // anything else gets there, so visit out of order, doing them all now.
            makeGlobalInitializers(node->getAsAggregate()->getSequence());

            // Initializers are done, don't want to visit again, but functions and link objects need to be processed,
            // so do them manually.
            visitFunctions(node->getAsAggregate()->getSequence());

            return false;
        }

        return true;
    }
    case Pglslang::EOpLinkerObjects:
    {
        if (visit == Pglslang::EvPreVisit)
            linkageOnly = true;
        else
            linkageOnly = false;

        return true;
    }
    case Pglslang::EOpComma:
    {
        // processing from left to right naturally leaves the right-most
        // lying around in the access chain
        Pglslang::TIntermSequence& glslangOperands = node->getSequence();
        for (int i = 0; i < (int)glslangOperands.size(); ++i)
            glslangOperands[i]->traverse(this);

        return false;
    }
    case Pglslang::EOpFunction:
        if (visit == Pglslang::EvPreVisit) {
            if (isShaderEntryPoint(node)) {
                inEntryPoint = true;
                builder.setBuildPoint(shaderEntry->getLastBlock());
                currentFunction = shaderEntry;
            } else {
                handleFunctionEntry(node);
            }
        } else {
            if (inEntryPoint)
                entryPointTerminated = true;
            builder.leaveFunction();
            inEntryPoint = false;
        }

        return true;
    case Pglslang::EOpParameters:
        // Parameters will have been consumed by EOpFunction processing, but not
        // the body, so we still visited the function node's children, making this
        // child redundant.
        return false;
    case Pglslang::EOpFunctionCall:
    {
        builder.setLine(node->getLoc().line, node->getLoc().getFilename());
        if (node->isUserDefined())
            result = handleUserFunctionCall(node);
        // assert(result);  // this can happen for bad shaders because the call graph completeness checking is not yet done
        if (result) {
            builder.clearAccessChain();
            builder.setAccessChainRValue(result);
        } else
            logger->missingFunctionality("missing user function; linker needs to catch that");

        return false;
    }
    case Pglslang::EOpConstructMat2x2:
    case Pglslang::EOpConstructMat2x3:
    case Pglslang::EOpConstructMat2x4:
    case Pglslang::EOpConstructMat3x2:
    case Pglslang::EOpConstructMat3x3:
    case Pglslang::EOpConstructMat3x4:
    case Pglslang::EOpConstructMat4x2:
    case Pglslang::EOpConstructMat4x3:
    case Pglslang::EOpConstructMat4x4:
    case Pglslang::EOpConstructDMat2x2:
    case Pglslang::EOpConstructDMat2x3:
    case Pglslang::EOpConstructDMat2x4:
    case Pglslang::EOpConstructDMat3x2:
    case Pglslang::EOpConstructDMat3x3:
    case Pglslang::EOpConstructDMat3x4:
    case Pglslang::EOpConstructDMat4x2:
    case Pglslang::EOpConstructDMat4x3:
    case Pglslang::EOpConstructDMat4x4:
    case Pglslang::EOpConstructIMat2x2:
    case Pglslang::EOpConstructIMat2x3:
    case Pglslang::EOpConstructIMat2x4:
    case Pglslang::EOpConstructIMat3x2:
    case Pglslang::EOpConstructIMat3x3:
    case Pglslang::EOpConstructIMat3x4:
    case Pglslang::EOpConstructIMat4x2:
    case Pglslang::EOpConstructIMat4x3:
    case Pglslang::EOpConstructIMat4x4:
    case Pglslang::EOpConstructUMat2x2:
    case Pglslang::EOpConstructUMat2x3:
    case Pglslang::EOpConstructUMat2x4:
    case Pglslang::EOpConstructUMat3x2:
    case Pglslang::EOpConstructUMat3x3:
    case Pglslang::EOpConstructUMat3x4:
    case Pglslang::EOpConstructUMat4x2:
    case Pglslang::EOpConstructUMat4x3:
    case Pglslang::EOpConstructUMat4x4:
    case Pglslang::EOpConstructBMat2x2:
    case Pglslang::EOpConstructBMat2x3:
    case Pglslang::EOpConstructBMat2x4:
    case Pglslang::EOpConstructBMat3x2:
    case Pglslang::EOpConstructBMat3x3:
    case Pglslang::EOpConstructBMat3x4:
    case Pglslang::EOpConstructBMat4x2:
    case Pglslang::EOpConstructBMat4x3:
    case Pglslang::EOpConstructBMat4x4:
    case Pglslang::EOpConstructF16Mat2x2:
    case Pglslang::EOpConstructF16Mat2x3:
    case Pglslang::EOpConstructF16Mat2x4:
    case Pglslang::EOpConstructF16Mat3x2:
    case Pglslang::EOpConstructF16Mat3x3:
    case Pglslang::EOpConstructF16Mat3x4:
    case Pglslang::EOpConstructF16Mat4x2:
    case Pglslang::EOpConstructF16Mat4x3:
    case Pglslang::EOpConstructF16Mat4x4:
        isMatrix = true;
        // fall through
    case Pglslang::EOpConstructFloat:
    case Pglslang::EOpConstructVec2:
    case Pglslang::EOpConstructVec3:
    case Pglslang::EOpConstructVec4:
    case Pglslang::EOpConstructDouble:
    case Pglslang::EOpConstructDVec2:
    case Pglslang::EOpConstructDVec3:
    case Pglslang::EOpConstructDVec4:
    case Pglslang::EOpConstructFloat16:
    case Pglslang::EOpConstructF16Vec2:
    case Pglslang::EOpConstructF16Vec3:
    case Pglslang::EOpConstructF16Vec4:
    case Pglslang::EOpConstructBool:
    case Pglslang::EOpConstructBVec2:
    case Pglslang::EOpConstructBVec3:
    case Pglslang::EOpConstructBVec4:
    case Pglslang::EOpConstructInt8:
    case Pglslang::EOpConstructI8Vec2:
    case Pglslang::EOpConstructI8Vec3:
    case Pglslang::EOpConstructI8Vec4:
    case Pglslang::EOpConstructUint8:
    case Pglslang::EOpConstructU8Vec2:
    case Pglslang::EOpConstructU8Vec3:
    case Pglslang::EOpConstructU8Vec4:
    case Pglslang::EOpConstructInt16:
    case Pglslang::EOpConstructI16Vec2:
    case Pglslang::EOpConstructI16Vec3:
    case Pglslang::EOpConstructI16Vec4:
    case Pglslang::EOpConstructUint16:
    case Pglslang::EOpConstructU16Vec2:
    case Pglslang::EOpConstructU16Vec3:
    case Pglslang::EOpConstructU16Vec4:
    case Pglslang::EOpConstructInt:
    case Pglslang::EOpConstructIVec2:
    case Pglslang::EOpConstructIVec3:
    case Pglslang::EOpConstructIVec4:
    case Pglslang::EOpConstructUint:
    case Pglslang::EOpConstructUVec2:
    case Pglslang::EOpConstructUVec3:
    case Pglslang::EOpConstructUVec4:
    case Pglslang::EOpConstructInt64:
    case Pglslang::EOpConstructI64Vec2:
    case Pglslang::EOpConstructI64Vec3:
    case Pglslang::EOpConstructI64Vec4:
    case Pglslang::EOpConstructUint64:
    case Pglslang::EOpConstructU64Vec2:
    case Pglslang::EOpConstructU64Vec3:
    case Pglslang::EOpConstructU64Vec4:
    case Pglslang::EOpConstructStruct:
    case Pglslang::EOpConstructTextureSampler:
    case Pglslang::EOpConstructReference:
    case Pglslang::EOpConstructCooperativeMatrix:
    {
        builder.setLine(node->getLoc().line, node->getLoc().getFilename());
        std::vector<Pspv::Id> arguments;
        translateArguments(*node, arguments, lvalueCoherentFlags);
        Pspv::Id constructed;
        if (node->getOp() == Pglslang::EOpConstructTextureSampler)
            constructed = builder.createOp(Pspv::OpSampledImage, resultType(), arguments);
        else if (node->getOp() == Pglslang::EOpConstructStruct ||
                 node->getOp() == Pglslang::EOpConstructCooperativeMatrix ||
                 node->getType().isArray()) {
            std::vector<Pspv::Id> constituents;
            for (int c = 0; c < (int)arguments.size(); ++c)
                constituents.push_back(arguments[c]);
            constructed = createCompositeConstruct(resultType(), constituents);
        } else if (isMatrix)
            constructed = builder.createMatrixConstructor(precision, arguments, resultType());
        else
            constructed = builder.createConstructor(precision, arguments, resultType());

        builder.clearAccessChain();
        builder.setAccessChainRValue(constructed);

        return false;
    }

    // These six are component-wise compares with component-wise results.
    // Forward on to createBinaryOperation(), requesting a vector result.
    case Pglslang::EOpLessThan:
    case Pglslang::EOpGreaterThan:
    case Pglslang::EOpLessThanEqual:
    case Pglslang::EOpGreaterThanEqual:
    case Pglslang::EOpVectorEqual:
    case Pglslang::EOpVectorNotEqual:
    {
        // Map the operation to a binary
        binOp = node->getOp();
        reduceComparison = false;
        switch (node->getOp()) {
        case Pglslang::EOpVectorEqual:     binOp = Pglslang::EOpVectorEqual;      break;
        case Pglslang::EOpVectorNotEqual:  binOp = Pglslang::EOpVectorNotEqual;   break;
        default:                          binOp = node->getOp();                break;
        }

        break;
    }
    case Pglslang::EOpMul:
        // component-wise matrix multiply
        binOp = Pglslang::EOpMul;
        break;
    case Pglslang::EOpOuterProduct:
        // two vectors multiplied to make a matrix
        binOp = Pglslang::EOpOuterProduct;
        break;
    case Pglslang::EOpDot:
    {
        // for scalar dot product, use multiply
        Pglslang::TIntermSequence& glslangOperands = node->getSequence();
        if (glslangOperands[0]->getAsTyped()->getVectorSize() == 1)
            binOp = Pglslang::EOpMul;
        break;
    }
    case Pglslang::EOpMod:
        // when an aggregate, this is the floating-point mod built-in function,
        // which can be emitted by the one in createBinaryOperation()
        binOp = Pglslang::EOpMod;
        break;

    case Pglslang::EOpEmitVertex:
    case Pglslang::EOpEndPrimitive:
    case Pglslang::EOpBarrier:
    case Pglslang::EOpMemoryBarrier:
    case Pglslang::EOpMemoryBarrierAtomicCounter:
    case Pglslang::EOpMemoryBarrierBuffer:
    case Pglslang::EOpMemoryBarrierImage:
    case Pglslang::EOpMemoryBarrierShared:
    case Pglslang::EOpGroupMemoryBarrier:
    case Pglslang::EOpDeviceMemoryBarrier:
    case Pglslang::EOpAllMemoryBarrierWithGroupSync:
    case Pglslang::EOpDeviceMemoryBarrierWithGroupSync:
    case Pglslang::EOpWorkgroupMemoryBarrier:
    case Pglslang::EOpWorkgroupMemoryBarrierWithGroupSync:
    case Pglslang::EOpSubgroupBarrier:
    case Pglslang::EOpSubgroupMemoryBarrier:
    case Pglslang::EOpSubgroupMemoryBarrierBuffer:
    case Pglslang::EOpSubgroupMemoryBarrierImage:
    case Pglslang::EOpSubgroupMemoryBarrierShared:
        noReturnValue = true;
        // These all have 0 operands and will naturally finish up in the code below for 0 operands
        break;

    case Pglslang::EOpAtomicAdd:
    case Pglslang::EOpAtomicMin:
    case Pglslang::EOpAtomicMax:
    case Pglslang::EOpAtomicAnd:
    case Pglslang::EOpAtomicOr:
    case Pglslang::EOpAtomicXor:
    case Pglslang::EOpAtomicExchange:
    case Pglslang::EOpAtomicCompSwap:
        atomic = true;
        break;

#ifndef GLSLANG_WEB
    case Pglslang::EOpAtomicStore:
        noReturnValue = true;
        // fallthrough
    case Pglslang::EOpAtomicLoad:
        atomic = true;
        break;

    case Pglslang::EOpAtomicCounterAdd:
    case Pglslang::EOpAtomicCounterSubtract:
    case Pglslang::EOpAtomicCounterMin:
    case Pglslang::EOpAtomicCounterMax:
    case Pglslang::EOpAtomicCounterAnd:
    case Pglslang::EOpAtomicCounterOr:
    case Pglslang::EOpAtomicCounterXor:
    case Pglslang::EOpAtomicCounterExchange:
    case Pglslang::EOpAtomicCounterCompSwap:
        builder.addExtension("SPV_KHR_shader_atomic_counter_ops");
        builder.addCapability(Pspv::CapabilityAtomicStorageOps);
        atomic = true;
        break;

    case Pglslang::EOpAbsDifference:
    case Pglslang::EOpAddSaturate:
    case Pglslang::EOpSubSaturate:
    case Pglslang::EOpAverage:
    case Pglslang::EOpAverageRounded:
    case Pglslang::EOpMul32x16:
        builder.addCapability(Pspv::CapabilityIntegerFunctions2INTEL);
        builder.addExtension("SPV_INTEL_shader_integer_functions2");
        binOp = node->getOp();
        break;

    case Pglslang::EOpIgnoreIntersectionNV:
    case Pglslang::EOpTerminateRayNV:
    case Pglslang::EOpTraceNV:
    case Pglslang::EOpExecuteCallableNV:
    case Pglslang::EOpWritePackedPrimitiveIndices4x8NV:
        noReturnValue = true;
        break;
    case Pglslang::EOpCooperativeMatrixLoad:
    case Pglslang::EOpCooperativeMatrixStore:
        noReturnValue = true;
        break;
    case Pglslang::EOpBeginInvocationInterlock:
    case Pglslang::EOpEndInvocationInterlock:
        builder.addExtension(Pspv::E_SPV_EXT_fragment_shader_interlock);
        noReturnValue = true;
        break;
#endif

    default:
        break;
    }

    //
    // See if it maps to a regular operation.
    //
    if (binOp != Pglslang::EOpNull) {
        Pglslang::TIntermTyped* left = node->getSequence()[0]->getAsTyped();
        Pglslang::TIntermTyped* right = node->getSequence()[1]->getAsTyped();
        assert(left && right);

        builder.clearAccessChain();
        left->traverse(this);
        Pspv::Id leftId = accessChainLoad(left->getType());

        builder.clearAccessChain();
        right->traverse(this);
        Pspv::Id rightId = accessChainLoad(right->getType());

        builder.setLine(node->getLoc().line, node->getLoc().getFilename());
        OpDecorations decorations = { precision,
                                      TranslateNoContractionDecoration(node->getType().getQualifier()),
                                      TranslateNonUniformDecoration(node->getType().getQualifier()) };
        result = createBinaryOperation(binOp, decorations,
                                       resultType(), leftId, rightId,
                                       left->getType().getBasicType(), reduceComparison);

        // code above should only make binOp that exists in createBinaryOperation
        assert(result != Pspv::NoResult);
        builder.clearAccessChain();
        builder.setAccessChainRValue(result);

        return false;
    }

    //
    // Create the list of operands.
    //
    Pglslang::TIntermSequence& glslangOperands = node->getSequence();
    std::vector<Pspv::Id> operands;
    std::vector<Pspv::IdImmediate> memoryAccessOperands;
    for (int arg = 0; arg < (int)glslangOperands.size(); ++arg) {
        // special case l-value operands; there are just a few
        bool lvalue = false;
        switch (node->getOp()) {
        case Pglslang::EOpModf:
            if (arg == 1)
                lvalue = true;
            break;

        case Pglslang::EOpAtomicAdd:
        case Pglslang::EOpAtomicMin:
        case Pglslang::EOpAtomicMax:
        case Pglslang::EOpAtomicAnd:
        case Pglslang::EOpAtomicOr:
        case Pglslang::EOpAtomicXor:
        case Pglslang::EOpAtomicExchange:
        case Pglslang::EOpAtomicCompSwap:
            if (arg == 0)
                lvalue = true;
            break;

#ifndef GLSLANG_WEB
        case Pglslang::EOpFrexp:
            if (arg == 1)
                lvalue = true;
            break;
        case Pglslang::EOpInterpolateAtSample:
        case Pglslang::EOpInterpolateAtOffset:
        case Pglslang::EOpInterpolateAtVertex:
            if (arg == 0) {
                lvalue = true;

                // Does it need a swizzle inversion?  If so, evaluation is inverted;
                // operate first on the swizzle base, then apply the swizzle.
                if (glslangOperands[0]->getAsOperator() &&
                    glslangOperands[0]->getAsOperator()->getOp() == Pglslang::EOpVectorSwizzle)
                    invertedType = convertGlslangToSpvType(glslangOperands[0]->getAsBinaryNode()->getLeft()->getType());
            }
            break;
        case Pglslang::EOpAtomicLoad:
        case Pglslang::EOpAtomicStore:
        case Pglslang::EOpAtomicCounterAdd:
        case Pglslang::EOpAtomicCounterSubtract:
        case Pglslang::EOpAtomicCounterMin:
        case Pglslang::EOpAtomicCounterMax:
        case Pglslang::EOpAtomicCounterAnd:
        case Pglslang::EOpAtomicCounterOr:
        case Pglslang::EOpAtomicCounterXor:
        case Pglslang::EOpAtomicCounterExchange:
        case Pglslang::EOpAtomicCounterCompSwap:
            if (arg == 0)
                lvalue = true;
            break;
        case Pglslang::EOpAddCarry:
        case Pglslang::EOpSubBorrow:
            if (arg == 2)
                lvalue = true;
            break;
        case Pglslang::EOpUMulExtended:
        case Pglslang::EOpIMulExtended:
            if (arg >= 2)
                lvalue = true;
            break;
        case Pglslang::EOpCooperativeMatrixLoad:
            if (arg == 0 || arg == 1)
                lvalue = true;
            break;
        case Pglslang::EOpCooperativeMatrixStore:
            if (arg == 1)
                lvalue = true;
            break;
#endif
        default:
            break;
        }
        builder.clearAccessChain();
        if (invertedType != Pspv::NoType && arg == 0)
            glslangOperands[0]->getAsBinaryNode()->getLeft()->traverse(this);
        else
            glslangOperands[arg]->traverse(this);

#ifndef GLSLANG_WEB
        if (node->getOp() == Pglslang::EOpCooperativeMatrixLoad ||
            node->getOp() == Pglslang::EOpCooperativeMatrixStore) {

            if (arg == 1) {
                // fold "element" parameter into the access chain
                Pspv::Builder::AccessChain save = builder.getAccessChain();
                builder.clearAccessChain();
                glslangOperands[2]->traverse(this);

                Pspv::Id elementId = accessChainLoad(glslangOperands[2]->getAsTyped()->getType());

                builder.setAccessChain(save);

                // Point to the first element of the array.
                builder.accessChainPush(elementId, TranslateCoherent(glslangOperands[arg]->getAsTyped()->getType()),
                                                   glslangOperands[arg]->getAsTyped()->getType().getBufferReferenceAlignment());

                Pspv::Builder::AccessChain::CoherentFlags coherentFlags = builder.getAccessChain().coherentFlags;
                unsigned int alignment = builder.getAccessChain().alignment;

                int memoryAccess = TranslateMemoryAccess(coherentFlags);
                if (node->getOp() == Pglslang::EOpCooperativeMatrixLoad)
                    memoryAccess &= ~Pspv::MemoryAccessMakePointerAvailableKHRMask;
                if (node->getOp() == Pglslang::EOpCooperativeMatrixStore)
                    memoryAccess &= ~Pspv::MemoryAccessMakePointerVisibleKHRMask;
                if (builder.getStorageClass(builder.getAccessChain().base) == Pspv::StorageClassPhysicalStorageBufferEXT) {
                    memoryAccess = (Pspv::MemoryAccessMask)(memoryAccess | Pspv::MemoryAccessAlignedMask);
                }

                memoryAccessOperands.push_back(Pspv::IdImmediate(false, memoryAccess));

                if (memoryAccess & Pspv::MemoryAccessAlignedMask) {
                    memoryAccessOperands.push_back(Pspv::IdImmediate(false, alignment));
                }

                if (memoryAccess & (Pspv::MemoryAccessMakePointerAvailableKHRMask | Pspv::MemoryAccessMakePointerVisibleKHRMask)) {
                    memoryAccessOperands.push_back(Pspv::IdImmediate(true, builder.makeUintConstant(TranslateMemoryScope(coherentFlags))));
                }
            } else if (arg == 2) {
                continue;
            }
        }
#endif

        if (lvalue) {
            operands.push_back(builder.accessChainGetLValue());
            lvalueCoherentFlags = builder.getAccessChain().coherentFlags;
            lvalueCoherentFlags |= TranslateCoherent(glslangOperands[arg]->getAsTyped()->getType());
        } else {
            builder.setLine(node->getLoc().line, node->getLoc().getFilename());
            operands.push_back(accessChainLoad(glslangOperands[arg]->getAsTyped()->getType()));
        }
    }

    builder.setLine(node->getLoc().line, node->getLoc().getFilename());
#ifndef GLSLANG_WEB
    if (node->getOp() == Pglslang::EOpCooperativeMatrixLoad) {
        std::vector<Pspv::IdImmediate> idImmOps;

        idImmOps.push_back(Pspv::IdImmediate(true, operands[1])); // buf
        idImmOps.push_back(Pspv::IdImmediate(true, operands[2])); // stride
        idImmOps.push_back(Pspv::IdImmediate(true, operands[3])); // colMajor
        idImmOps.insert(idImmOps.end(), memoryAccessOperands.begin(), memoryAccessOperands.end());
        // get the pointee type
        Pspv::Id typeId = builder.getContainedTypeId(builder.getTypeId(operands[0]));
        assert(builder.isCooperativeMatrixType(typeId));
        // do the op
        Pspv::Id result = builder.createOp(Pspv::OpCooperativeMatrixLoadNV, typeId, idImmOps);
        // store the result to the pointer (out param 'm')
        builder.createStore(result, operands[0]);
        result = 0;
    } else if (node->getOp() == Pglslang::EOpCooperativeMatrixStore) {
        std::vector<Pspv::IdImmediate> idImmOps;

        idImmOps.push_back(Pspv::IdImmediate(true, operands[1])); // buf
        idImmOps.push_back(Pspv::IdImmediate(true, operands[0])); // object
        idImmOps.push_back(Pspv::IdImmediate(true, operands[2])); // stride
        idImmOps.push_back(Pspv::IdImmediate(true, operands[3])); // colMajor
        idImmOps.insert(idImmOps.end(), memoryAccessOperands.begin(), memoryAccessOperands.end());

        builder.createNoResultOp(Pspv::OpCooperativeMatrixStoreNV, idImmOps);
        result = 0;
    } else
#endif
    if (atomic) {
        // Handle all atomics
        result = createAtomicOperation(node->getOp(), precision, resultType(), operands, node->getBasicType(), lvalueCoherentFlags);
    } else {
        // Pass through to generic operations.
        switch (glslangOperands.size()) {
        case 0:
            result = createNoArgOperation(node->getOp(), precision, resultType());
            break;
        case 1:
            {
                OpDecorations decorations = { precision, 
                                              TranslateNoContractionDecoration(node->getType().getQualifier()),
                                              TranslateNonUniformDecoration(node->getType().getQualifier()) };
                result = createUnaryOperation(
                    node->getOp(), decorations,
                    resultType(), operands.front(),
                    glslangOperands[0]->getAsTyped()->getBasicType(), lvalueCoherentFlags);
            }
            break;
        default:
            result = createMiscOperation(node->getOp(), precision, resultType(), operands, node->getBasicType());
            break;
        }
        if (invertedType)
            result = createInvertedSwizzle(precision, *glslangOperands[0]->getAsBinaryNode(), result);
    }

    if (noReturnValue)
        return false;

    if (! result) {
        logger->missingFunctionality("unknown glslang aggregate");
        return true;  // pick up a child as a placeholder operand
    } else {
        builder.clearAccessChain();
        builder.setAccessChainRValue(result);
        return false;
    }
}

// This path handles both if-then-else and ?:
// The if-then-else has a node type of void, while
// ?: has either a void or a non-void node type
//
// Leaving the result, when not void:
// GLSL only has r-values as the result of a :?, but
// if we have an l-value, that can be more efficient if it will
// become the base of a complex r-value expression, because the
// next layer copies r-values into memory to use the access-chain mechanism
bool TGlslangToSpvTraverser::visitSelection(Pglslang::TVisit /* visit */, Pglslang::TIntermSelection* node)
{
    // see if OpSelect can handle it
    const auto isOpSelectable = [&]() {
        if (node->getBasicType() == Pglslang::EbtVoid)
            return false;
        // OpSelect can do all other types starting with SPV 1.4
        if (glslangIntermediate->getSpv().spv < Pglslang::EShTargetSpv_1_4) {
            // pre-1.4, only scalars and vectors can be handled
            if ((!node->getType().isScalar() && !node->getType().isVector()))
                return false;
        }
        return true;
    };

    // See if it simple and safe, or required, to execute both sides.
    // Crucially, side effects must be either semantically required or avoided,
    // and there are performance trade-offs.
    // Return true if required or a good idea (and safe) to execute both sides,
    // false otherwise.
    const auto bothSidesPolicy = [&]() -> bool {
        // do we have both sides?
        if (node->getTrueBlock()  == nullptr ||
            node->getFalseBlock() == nullptr)
            return false;

        // required? (unless we write additional code to look for side effects
        // and make performance trade-offs if none are present)
        if (!node->getShortCircuit())
            return true;

        // if not required to execute both, decide based on performance/practicality...

        if (!isOpSelectable())
            return false;

        assert(node->getType() == node->getTrueBlock() ->getAsTyped()->getType() &&
               node->getType() == node->getFalseBlock()->getAsTyped()->getType());

        // return true if a single operand to ? : is okay for OpSelect
        const auto operandOkay = [](Pglslang::TIntermTyped* node) {
            return node->getAsSymbolNode() || node->getType().getQualifier().isConstant();
        };

        return operandOkay(node->getTrueBlock() ->getAsTyped()) &&
               operandOkay(node->getFalseBlock()->getAsTyped());
    };

    Pspv::Id result = Pspv::NoResult; // upcoming result selecting between trueValue and falseValue
    // emit the condition before doing anything with selection
    node->getCondition()->traverse(this);
    Pspv::Id condition = accessChainLoad(node->getCondition()->getType());

    // Find a way of executing both sides and selecting the right result.
    const auto executeBothSides = [&]() -> void {
        // execute both sides
        node->getTrueBlock()->traverse(this);
        Pspv::Id trueValue = accessChainLoad(node->getTrueBlock()->getAsTyped()->getType());
        node->getFalseBlock()->traverse(this);
        Pspv::Id falseValue = accessChainLoad(node->getTrueBlock()->getAsTyped()->getType());

        builder.setLine(node->getLoc().line, node->getLoc().getFilename());

        // done if void
        if (node->getBasicType() == Pglslang::EbtVoid)
            return;

        // emit code to select between trueValue and falseValue

        // see if OpSelect can handle it
        if (isOpSelectable()) {
            // Emit OpSelect for this selection.

            // smear condition to vector, if necessary (AST is always scalar)
            // Before 1.4, smear like for mix(), starting with 1.4, keep it scalar
            if (glslangIntermediate->getSpv().spv < Pglslang::EShTargetSpv_1_4 && builder.isVector(trueValue)) {
                condition = builder.smearScalar(Pspv::NoPrecision, condition, 
                                                builder.makeVectorType(builder.makeBoolType(),
                                                                       builder.getNumComponents(trueValue)));
            }

            // OpSelect
            result = builder.createTriOp(Pspv::OpSelect,
                                         convertGlslangToSpvType(node->getType()), condition,
                                                                 trueValue, falseValue);

            builder.clearAccessChain();
            builder.setAccessChainRValue(result);
        } else {
            // We need control flow to select the result.
            // TODO: Once SPIR-V OpSelect allows arbitrary types, eliminate this path.
            result = builder.createVariable(Pspv::StorageClassFunction, convertGlslangToSpvType(node->getType()));

            // Selection control:
            const Pspv::SelectionControlMask control = TranslateSelectionControl(*node);

            // make an "if" based on the value created by the condition
            Pspv::Builder::If ifBuilder(condition, control, builder);

            // emit the "then" statement
            builder.createStore(trueValue, result);
            ifBuilder.makeBeginElse();
            // emit the "else" statement
            builder.createStore(falseValue, result);

            // finish off the control flow
            ifBuilder.makeEndIf();

            builder.clearAccessChain();
            builder.setAccessChainLValue(result);
        }
    };

    // Execute the one side needed, as per the condition
    const auto executeOneSide = [&]() {
        // Always emit control flow.
        if (node->getBasicType() != Pglslang::EbtVoid)
            result = builder.createVariable(Pspv::StorageClassFunction, convertGlslangToSpvType(node->getType()));

        // Selection control:
        const Pspv::SelectionControlMask control = TranslateSelectionControl(*node);

        // make an "if" based on the value created by the condition
        Pspv::Builder::If ifBuilder(condition, control, builder);

        // emit the "then" statement
        if (node->getTrueBlock() != nullptr) {
            node->getTrueBlock()->traverse(this);
            if (result != Pspv::NoResult)
                builder.createStore(accessChainLoad(node->getTrueBlock()->getAsTyped()->getType()), result);
        }

        if (node->getFalseBlock() != nullptr) {
            ifBuilder.makeBeginElse();
            // emit the "else" statement
            node->getFalseBlock()->traverse(this);
            if (result != Pspv::NoResult)
                builder.createStore(accessChainLoad(node->getFalseBlock()->getAsTyped()->getType()), result);
        }

        // finish off the control flow
        ifBuilder.makeEndIf();

        if (result != Pspv::NoResult) {
            builder.clearAccessChain();
            builder.setAccessChainLValue(result);
        }
    };

    // Try for OpSelect (or a requirement to execute both sides)
    if (bothSidesPolicy()) {
        SpecConstantOpModeGuard spec_constant_op_mode_setter(&builder);
        if (node->getType().getQualifier().isSpecConstant())
            spec_constant_op_mode_setter.turnOnSpecConstantOpMode();
        executeBothSides();
    } else
        executeOneSide();

    return false;
}

bool TGlslangToSpvTraverser::visitSwitch(Pglslang::TVisit /* visit */, Pglslang::TIntermSwitch* node)
{
    // emit and get the condition before doing anything with switch
    node->getCondition()->traverse(this);
    Pspv::Id selector = accessChainLoad(node->getCondition()->getAsTyped()->getType());

    // Selection control:
    const Pspv::SelectionControlMask control = TranslateSwitchControl(*node);

    // browse the children to sort out code segments
    int defaultSegment = -1;
    std::vector<TIntermNode*> codeSegments;
    Pglslang::TIntermSequence& sequence = node->getBody()->getSequence();
    std::vector<int> caseValues;
    std::vector<int> valueIndexToSegment(sequence.size());  // note: probably not all are used, it is an overestimate
    for (Pglslang::TIntermSequence::iterator c = sequence.begin(); c != sequence.end(); ++c) {
        TIntermNode* child = *c;
        if (child->getAsBranchNode() && child->getAsBranchNode()->getFlowOp() == Pglslang::EOpDefault)
            defaultSegment = (int)codeSegments.size();
        else if (child->getAsBranchNode() && child->getAsBranchNode()->getFlowOp() == Pglslang::EOpCase) {
            valueIndexToSegment[caseValues.size()] = (int)codeSegments.size();
            caseValues.push_back(child->getAsBranchNode()->getExpression()->getAsConstantUnion()->getConstArray()[0].getIConst());
        } else
            codeSegments.push_back(child);
    }

    // handle the case where the last code segment is missing, due to no code
    // statements between the last case and the end of the switch statement
    if ((caseValues.size() && (int)codeSegments.size() == valueIndexToSegment[caseValues.size() - 1]) ||
        (int)codeSegments.size() == defaultSegment)
        codeSegments.push_back(nullptr);

    // make the switch statement
    std::vector<Pspv::Block*> segmentBlocks; // returned, as the blocks allocated in the call
    builder.makeSwitch(selector, control, (int)codeSegments.size(), caseValues, valueIndexToSegment, defaultSegment, segmentBlocks);

    // emit all the code in the segments
    breakForLoop.push(false);
    for (unsigned int s = 0; s < codeSegments.size(); ++s) {
        builder.nextSwitchSegment(segmentBlocks, s);
        if (codeSegments[s])
            codeSegments[s]->traverse(this);
        else
            builder.addSwitchBreak();
    }
    breakForLoop.pop();

    builder.endSwitch(segmentBlocks);

    return false;
}

void TGlslangToSpvTraverser::visitConstantUnion(Pglslang::TIntermConstantUnion* node)
{
    int nextConst = 0;
    Pspv::Id constant = createSpvConstantFromConstUnionArray(node->getType(), node->getConstArray(), nextConst, false);

    builder.clearAccessChain();
    builder.setAccessChainRValue(constant);
}

bool TGlslangToSpvTraverser::visitLoop(Pglslang::TVisit /* visit */, Pglslang::TIntermLoop* node)
{
    auto blocks = builder.makeNewLoop();
    builder.createBranch(&blocks.head);

    // Loop control:
    std::vector<unsigned int> operands;
    const Pspv::LoopControlMask control = TranslateLoopControl(*node, operands);

    // Spec requires back edges to target header blocks, and every header block
    // must dominate its merge block.  Make a header block first to ensure these
    // conditions are met.  By definition, it will contain OpLoopMerge, followed
    // by a block-ending branch.  But we don't want to put any other body/test
    // instructions in it, since the body/test may have arbitrary instructions,
    // including merges of its own.
    builder.setLine(node->getLoc().line, node->getLoc().getFilename());
    builder.setBuildPoint(&blocks.head);
    builder.createLoopMerge(&blocks.merge, &blocks.continue_target, control, operands);
    if (node->testFirst() && node->getTest()) {
        Pspv::Block& test = builder.makeNewBlock();
        builder.createBranch(&test);

        builder.setBuildPoint(&test);
        node->getTest()->traverse(this);
        Pspv::Id condition = accessChainLoad(node->getTest()->getType());
        builder.createConditionalBranch(condition, &blocks.body, &blocks.merge);

        builder.setBuildPoint(&blocks.body);
        breakForLoop.push(true);
        if (node->getBody())
            node->getBody()->traverse(this);
        builder.createBranch(&blocks.continue_target);
        breakForLoop.pop();

        builder.setBuildPoint(&blocks.continue_target);
        if (node->getTerminal())
            node->getTerminal()->traverse(this);
        builder.createBranch(&blocks.head);
    } else {
        builder.setLine(node->getLoc().line, node->getLoc().getFilename());
        builder.createBranch(&blocks.body);

        breakForLoop.push(true);
        builder.setBuildPoint(&blocks.body);
        if (node->getBody())
            node->getBody()->traverse(this);
        builder.createBranch(&blocks.continue_target);
        breakForLoop.pop();

        builder.setBuildPoint(&blocks.continue_target);
        if (node->getTerminal())
            node->getTerminal()->traverse(this);
        if (node->getTest()) {
            node->getTest()->traverse(this);
            Pspv::Id condition =
                accessChainLoad(node->getTest()->getType());
            builder.createConditionalBranch(condition, &blocks.head, &blocks.merge);
        } else {
            // TODO: unless there was a break/return/discard instruction
            // somewhere in the body, this is an infinite loop, so we should
            // issue a warning.
            builder.createBranch(&blocks.head);
        }
    }
    builder.setBuildPoint(&blocks.merge);
    builder.closeLoop();
    return false;
}

bool TGlslangToSpvTraverser::visitBranch(Pglslang::TVisit /* visit */, Pglslang::TIntermBranch* node)
{
    if (node->getExpression())
        node->getExpression()->traverse(this);

    builder.setLine(node->getLoc().line, node->getLoc().getFilename());

    switch (node->getFlowOp()) {
    case Pglslang::EOpKill:
        builder.makeDiscard();
        break;
    case Pglslang::EOpBreak:
        if (breakForLoop.top())
            builder.createLoopExit();
        else
            builder.addSwitchBreak();
        break;
    case Pglslang::EOpContinue:
        builder.createLoopContinue();
        break;
    case Pglslang::EOpReturn:
        if (node->getExpression()) {
            const Pglslang::TType& glslangReturnType = node->getExpression()->getType();
            Pspv::Id returnId = accessChainLoad(glslangReturnType);
            if (builder.getTypeId(returnId) != currentFunction->getReturnType()) {
                builder.clearAccessChain();
                Pspv::Id copyId = builder.createVariable(Pspv::StorageClassFunction, currentFunction->getReturnType());
                builder.setAccessChainLValue(copyId);
                multiTypeStore(glslangReturnType, returnId);
                returnId = builder.createLoad(copyId);
            }
            builder.makeReturn(false, returnId);
        } else
            builder.makeReturn(false);

        builder.clearAccessChain();
        break;

#ifndef GLSLANG_WEB
    case Pglslang::EOpDemote:
        builder.createNoResultOp(Pspv::OpDemoteToHelperInvocationEXT);
        builder.addExtension(Pspv::E_SPV_EXT_demote_to_helper_invocation);
        builder.addCapability(Pspv::CapabilityDemoteToHelperInvocationEXT);
        break;
#endif

    default:
        assert(0);
        break;
    }

    return false;
}

Pspv::Id TGlslangToSpvTraverser::createSpvVariable(const Pglslang::TIntermSymbol* node, Pspv::Id forcedType)
{
    // First, steer off constants, which are not SPIR-V variables, but
    // can still have a mapping to a SPIR-V Id.
    // This includes specialization constants.
    if (node->getQualifier().isConstant()) {
        Pspv::Id result = createSpvConstant(*node);
        if (result != Pspv::NoResult)
            return result;
    }

    // Now, handle actual variables
    Pspv::StorageClass storageClass = TranslateStorageClass(node->getType());
    Pspv::Id spvType = forcedType == Pspv::NoType ? convertGlslangToSpvType(node->getType())
                                                : forcedType;

    const bool contains16BitType = node->getType().contains16BitFloat() ||
                                   node->getType().contains16BitInt();
    if (contains16BitType) {
        switch (storageClass) {
        case Pspv::StorageClassInput:
        case Pspv::StorageClassOutput:
            builder.addIncorporatedExtension(Pspv::E_SPV_KHR_16bit_storage, Pspv::Spv_1_3);
            builder.addCapability(Pspv::CapabilityStorageInputOutput16);
            break;
        case Pspv::StorageClassUniform:
            builder.addIncorporatedExtension(Pspv::E_SPV_KHR_16bit_storage, Pspv::Spv_1_3);
            if (node->getType().getQualifier().storage == Pglslang::EvqBuffer)
                builder.addCapability(Pspv::CapabilityStorageUniformBufferBlock16);
            else
                builder.addCapability(Pspv::CapabilityStorageUniform16);
            break;
#ifndef GLSLANG_WEB
        case Pspv::StorageClassPushConstant:
            builder.addIncorporatedExtension(Pspv::E_SPV_KHR_16bit_storage, Pspv::Spv_1_3);
            builder.addCapability(Pspv::CapabilityStoragePushConstant16);
            break;
        case Pspv::StorageClassStorageBuffer:
        case Pspv::StorageClassPhysicalStorageBufferEXT:
            builder.addIncorporatedExtension(Pspv::E_SPV_KHR_16bit_storage, Pspv::Spv_1_3);
            builder.addCapability(Pspv::CapabilityStorageUniformBufferBlock16);
            break;
#endif
        default:
            if (node->getType().contains16BitFloat())
                builder.addCapability(Pspv::CapabilityFloat16);
            if (node->getType().contains16BitInt())
                builder.addCapability(Pspv::CapabilityInt16);
            break;
        }
    }

    if (node->getType().contains8BitInt()) {
        if (storageClass == Pspv::StorageClassPushConstant) {
            builder.addIncorporatedExtension(Pspv::E_SPV_KHR_8bit_storage, Pspv::Spv_1_5);
            builder.addCapability(Pspv::CapabilityStoragePushConstant8);
        } else if (storageClass == Pspv::StorageClassUniform) {
            builder.addIncorporatedExtension(Pspv::E_SPV_KHR_8bit_storage, Pspv::Spv_1_5);
            builder.addCapability(Pspv::CapabilityUniformAndStorageBuffer8BitAccess);
        } else if (storageClass == Pspv::StorageClassStorageBuffer) {
            builder.addIncorporatedExtension(Pspv::E_SPV_KHR_8bit_storage, Pspv::Spv_1_5);
            builder.addCapability(Pspv::CapabilityStorageBuffer8BitAccess);
        } else {
            builder.addCapability(Pspv::CapabilityInt8);
        }
    }

    const char* name = node->getName().c_str();
    if (Pglslang::IsAnonymous(name))
        name = "";

    return builder.createVariable(storageClass, spvType, name);
}

// Return type Id of the sampled type.
Pspv::Id TGlslangToSpvTraverser::getSampledType(const Pglslang::TSampler& sampler)
{
    switch (sampler.type) {
        case Pglslang::EbtInt:      return builder.makeIntType(32);
        case Pglslang::EbtUint:     return builder.makeUintType(32);
        case Pglslang::EbtFloat:    return builder.makeFloatType(32);
#ifndef GLSLANG_WEB
        case Pglslang::EbtFloat16:
            builder.addExtension(Pspv::E_SPV_AMD_gpu_shader_half_float_fetch);
            builder.addCapability(Pspv::CapabilityFloat16ImageAMD);
            return builder.makeFloatType(16);
#endif
        default:
            assert(0);
            return builder.makeFloatType(32);
    }
}

// If node is a swizzle operation, return the type that should be used if
// the swizzle base is first consumed by another operation, before the swizzle
// is applied.
Pspv::Id TGlslangToSpvTraverser::getInvertedSwizzleType(const Pglslang::TIntermTyped& node)
{
    if (node.getAsOperator() &&
        node.getAsOperator()->getOp() == Pglslang::EOpVectorSwizzle)
        return convertGlslangToSpvType(node.getAsBinaryNode()->getLeft()->getType());
    else
        return Pspv::NoType;
}

// When inverting a swizzle with a parent op, this function
// will apply the swizzle operation to a completed parent operation.
Pspv::Id TGlslangToSpvTraverser::createInvertedSwizzle(Pspv::Decoration precision, const Pglslang::TIntermTyped& node, Pspv::Id parentResult)
{
    std::vector<unsigned> swizzle;
    convertSwizzle(*node.getAsBinaryNode()->getRight()->getAsAggregate(), swizzle);
    return builder.createRvalueSwizzle(precision, convertGlslangToSpvType(node.getType()), parentResult, swizzle);
}

// Convert a glslang AST swizzle node to a swizzle vector for building SPIR-V.
void TGlslangToSpvTraverser::convertSwizzle(const Pglslang::TIntermAggregate& node, std::vector<unsigned>& swizzle)
{
    const Pglslang::TIntermSequence& swizzleSequence = node.getSequence();
    for (int i = 0; i < (int)swizzleSequence.size(); ++i)
        swizzle.push_back(swizzleSequence[i]->getAsConstantUnion()->getConstArray()[0].getIConst());
}

// Convert from a glslang type to an SPV type, by calling into a
// recursive version of this function. This establishes the inherited
// layout state rooted from the top-level type.
Pspv::Id TGlslangToSpvTraverser::convertGlslangToSpvType(const Pglslang::TType& type, bool forwardReferenceOnly)
{
    return convertGlslangToSpvType(type, getExplicitLayout(type), type.getQualifier(), false, forwardReferenceOnly);
}

// Do full recursive conversion of an arbitrary glslang type to a SPIR-V Id.
// explicitLayout can be kept the same throughout the hierarchical recursive walk.
// Mutually recursive with convertGlslangStructToSpvType().
Pspv::Id TGlslangToSpvTraverser::convertGlslangToSpvType(const Pglslang::TType& type,
    Pglslang::TLayoutPacking explicitLayout, const Pglslang::TQualifier& qualifier,
    bool lastBufferBlockMember, bool forwardReferenceOnly)
{
    Pspv::Id spvType = Pspv::NoResult;

    switch (type.getBasicType()) {
    case Pglslang::EbtVoid:
        spvType = builder.makeVoidType();
        assert (! type.isArray());
        break;
    case Pglslang::EbtBool:
        // "transparent" bool doesn't exist in SPIR-V.  The GLSL convention is
        // a 32-bit int where non-0 means true.
        if (explicitLayout != Pglslang::ElpNone)
            spvType = builder.makeUintType(32);
        else
            spvType = builder.makeBoolType();
        break;
    case Pglslang::EbtInt:
        spvType = builder.makeIntType(32);
        break;
    case Pglslang::EbtUint:
        spvType = builder.makeUintType(32);
        break;
    case Pglslang::EbtFloat:
        spvType = builder.makeFloatType(32);
        break;
#ifndef GLSLANG_WEB
    case Pglslang::EbtDouble:
        spvType = builder.makeFloatType(64);
        break;
    case Pglslang::EbtFloat16:
        spvType = builder.makeFloatType(16);
        break;
    case Pglslang::EbtInt8:
        spvType = builder.makeIntType(8);
        break;
    case Pglslang::EbtUint8:
        spvType = builder.makeUintType(8);
        break;
    case Pglslang::EbtInt16:
        spvType = builder.makeIntType(16);
        break;
    case Pglslang::EbtUint16:
        spvType = builder.makeUintType(16);
        break;
    case Pglslang::EbtInt64:
        spvType = builder.makeIntType(64);
        break;
    case Pglslang::EbtUint64:
        spvType = builder.makeUintType(64);
        break;
    case Pglslang::EbtAtomicUint:
        builder.addCapability(Pspv::CapabilityAtomicStorage);
        spvType = builder.makeUintType(32);
        break;
    case Pglslang::EbtAccStructNV:
        spvType = builder.makeAccelerationStructureNVType();
        break;
    case Pglslang::EbtReference:
        {
            // Make the forward pointer, then recurse to convert the structure type, then
            // patch up the forward pointer with a real pointer type.
            if (forwardPointers.find(type.getReferentType()) == forwardPointers.end()) {
                Pspv::Id forwardId = builder.makeForwardPointer(Pspv::StorageClassPhysicalStorageBufferEXT);
                forwardPointers[type.getReferentType()] = forwardId;
            }
            spvType = forwardPointers[type.getReferentType()];
            if (!forwardReferenceOnly) {
                Pspv::Id referentType = convertGlslangToSpvType(*type.getReferentType());
                builder.makePointerFromForwardPointer(Pspv::StorageClassPhysicalStorageBufferEXT,
                                                      forwardPointers[type.getReferentType()],
                                                      referentType);
            }
        }
        break;
#endif
    case Pglslang::EbtSampler:
        {
            const Pglslang::TSampler& sampler = type.getSampler();
            if (sampler.isPureSampler()) {
                spvType = builder.makeSamplerType();
            } else {
                // an image is present, make its type
                spvType = builder.makeImageType(getSampledType(sampler), TranslateDimensionality(sampler),
                                                sampler.isShadow(), sampler.isArrayed(), sampler.isMultiSample(),
                                                sampler.isImageClass() ? 2 : 1, TranslateImageFormat(type));
                if (sampler.isCombined()) {
                    // already has both image and sampler, make the combined type
                    spvType = builder.makeSampledImageType(spvType);
                }
            }
        }
        break;
    case Pglslang::EbtStruct:
    case Pglslang::EbtBlock:
        {
            // If we've seen this struct type, return it
            const Pglslang::TTypeList* glslangMembers = type.getStruct();

            // Try to share structs for different layouts, but not yet for other
            // kinds of qualification (primarily not yet including interpolant qualification).
            if (! HasNonLayoutQualifiers(type, qualifier))
                spvType = structMap[explicitLayout][qualifier.layoutMatrix][glslangMembers];
            if (spvType != Pspv::NoResult)
                break;

            // else, we haven't seen it...
            if (type.getBasicType() == Pglslang::EbtBlock)
                memberRemapper[glslangTypeToIdMap[glslangMembers]].resize(glslangMembers->size());
            spvType = convertGlslangStructToSpvType(type, glslangMembers, explicitLayout, qualifier);
        }
        break;
    default:
        assert(0);
        break;
    }

    if (type.isMatrix())
        spvType = builder.makeMatrixType(spvType, type.getMatrixCols(), type.getMatrixRows());
    else {
        // If this variable has a vector element count greater than 1, create a SPIR-V vector
        if (type.getVectorSize() > 1)
            spvType = builder.makeVectorType(spvType, type.getVectorSize());
    }

    if (type.isCoopMat()) {
        builder.addCapability(Pspv::CapabilityCooperativeMatrixNV);
        builder.addExtension(Pspv::E_SPV_NV_cooperative_matrix);
        if (type.getBasicType() == Pglslang::EbtFloat16)
            builder.addCapability(Pspv::CapabilityFloat16);
        if (type.getBasicType() == Pglslang::EbtUint8 ||
            type.getBasicType() == Pglslang::EbtInt8) {
            builder.addCapability(Pspv::CapabilityInt8);
        }

        Pspv::Id scope = makeArraySizeId(*type.getTypeParameters(), 1);
        Pspv::Id rows = makeArraySizeId(*type.getTypeParameters(), 2);
        Pspv::Id cols = makeArraySizeId(*type.getTypeParameters(), 3);

        spvType = builder.makeCooperativeMatrixType(spvType, scope, rows, cols);
    }

    if (type.isArray()) {
        int stride = 0;  // keep this 0 unless doing an explicit layout; 0 will mean no decoration, no stride

        // Do all but the outer dimension
        if (type.getArraySizes()->getNumDims() > 1) {
            // We need to decorate array strides for types needing explicit layout, except blocks.
            if (explicitLayout != Pglslang::ElpNone && type.getBasicType() != Pglslang::EbtBlock) {
                // Use a dummy glslang type for querying internal strides of
                // arrays of arrays, but using just a one-dimensional array.
                Pglslang::TType simpleArrayType(type, 0); // deference type of the array
                while (simpleArrayType.getArraySizes()->getNumDims() > 1)
                    simpleArrayType.getArraySizes()->dereference();

                // Will compute the higher-order strides here, rather than making a whole
                // pile of types and doing repetitive recursion on their contents.
                stride = getArrayStride(simpleArrayType, explicitLayout, qualifier.layoutMatrix);
            }

            // make the arrays
            for (int dim = type.getArraySizes()->getNumDims() - 1; dim > 0; --dim) {
                spvType = builder.makeArrayType(spvType, makeArraySizeId(*type.getArraySizes(), dim), stride);
                if (stride > 0)
                    builder.addDecoration(spvType, Pspv::DecorationArrayStride, stride);
                stride *= type.getArraySizes()->getDimSize(dim);
            }
        } else {
            // single-dimensional array, and don't yet have stride

            // We need to decorate array strides for types needing explicit layout, except blocks.
            if (explicitLayout != Pglslang::ElpNone && type.getBasicType() != Pglslang::EbtBlock)
                stride = getArrayStride(type, explicitLayout, qualifier.layoutMatrix);
        }

        // Do the outer dimension, which might not be known for a runtime-sized array.
        // (Unsized arrays that survive through linking will be runtime-sized arrays)
        if (type.isSizedArray())
            spvType = builder.makeArrayType(spvType, makeArraySizeId(*type.getArraySizes(), 0), stride);
        else {
#ifndef GLSLANG_WEB
            if (!lastBufferBlockMember) {
                builder.addIncorporatedExtension("SPV_EXT_descriptor_indexing", Pspv::Spv_1_5);
                builder.addCapability(Pspv::CapabilityRuntimeDescriptorArrayEXT);
            }
#endif
            spvType = builder.makeRuntimeArray(spvType);
        }
        if (stride > 0)
            builder.addDecoration(spvType, Pspv::DecorationArrayStride, stride);
    }

    return spvType;
}

// TODO: this functionality should exist at a higher level, in creating the AST
//
// Identify interface members that don't have their required extension turned on.
//
bool TGlslangToSpvTraverser::filterMember(const Pglslang::TType& member)
{
#ifndef GLSLANG_WEB
    auto& extensions = glslangIntermediate->getRequestedExtensions();

    if (member.getFieldName() == "gl_SecondaryViewportMaskNV" &&
        extensions.find("GL_NV_stereo_view_rendering") == extensions.end())
        return true;
    if (member.getFieldName() == "gl_SecondaryPositionNV" &&
        extensions.find("GL_NV_stereo_view_rendering") == extensions.end())
        return true;

    if (glslangIntermediate->getStage() != EShLangMeshNV) {
        if (member.getFieldName() == "gl_ViewportMask" &&
            extensions.find("GL_NV_viewport_array2") == extensions.end())
            return true;
        if (member.getFieldName() == "gl_PositionPerViewNV" &&
            extensions.find("GL_NVX_multiview_per_view_attributes") == extensions.end())
            return true;
        if (member.getFieldName() == "gl_ViewportMaskPerViewNV" &&
            extensions.find("GL_NVX_multiview_per_view_attributes") == extensions.end())
            return true;
    }
#endif

    return false;
};

// Do full recursive conversion of a glslang structure (or block) type to a SPIR-V Id.
// explicitLayout can be kept the same throughout the hierarchical recursive walk.
// Mutually recursive with convertGlslangToSpvType().
Pspv::Id TGlslangToSpvTraverser::convertGlslangStructToSpvType(const Pglslang::TType& type,
                                                              const Pglslang::TTypeList* glslangMembers,
                                                              Pglslang::TLayoutPacking explicitLayout,
                                                              const Pglslang::TQualifier& qualifier)
{
    // Create a vector of struct types for SPIR-V to consume
    std::vector<Pspv::Id> spvMembers;
    int memberDelta = 0;  // how much the member's index changes from glslang to SPIR-V, normally 0, except sometimes for blocks
    std::vector<std::pair<Pglslang::TType*, Pglslang::TQualifier> > deferredForwardPointers;
    for (int i = 0; i < (int)glslangMembers->size(); i++) {
        Pglslang::TType& glslangMember = *(*glslangMembers)[i].type;
        if (glslangMember.hiddenMember()) {
            ++memberDelta;
            if (type.getBasicType() == Pglslang::EbtBlock)
                memberRemapper[glslangTypeToIdMap[glslangMembers]][i] = -1;
        } else {
            if (type.getBasicType() == Pglslang::EbtBlock) {
                if (filterMember(glslangMember)) {
                    memberDelta++;
                    memberRemapper[glslangTypeToIdMap[glslangMembers]][i] = -1;
                    continue;
                }
                memberRemapper[glslangTypeToIdMap[glslangMembers]][i] = i - memberDelta;
            }
            // modify just this child's view of the qualifier
            Pglslang::TQualifier memberQualifier = glslangMember.getQualifier();
            InheritQualifiers(memberQualifier, qualifier);

            // manually inherit location
            if (! memberQualifier.hasLocation() && qualifier.hasLocation())
                memberQualifier.layoutLocation = qualifier.layoutLocation;

            // recurse
            bool lastBufferBlockMember = qualifier.storage == Pglslang::EvqBuffer &&
                                         i == (int)glslangMembers->size() - 1;

            // Make forward pointers for any pointer members, and create a list of members to
            // convert to spirv types after creating the struct.
            if (glslangMember.isReference()) {
                if (forwardPointers.find(glslangMember.getReferentType()) == forwardPointers.end()) {
                    deferredForwardPointers.push_back(std::make_pair(&glslangMember, memberQualifier));
                }
                spvMembers.push_back(
                    convertGlslangToSpvType(glslangMember, explicitLayout, memberQualifier, lastBufferBlockMember, true));
            } else {
                spvMembers.push_back(
                    convertGlslangToSpvType(glslangMember, explicitLayout, memberQualifier, lastBufferBlockMember, false));
            }
        }
    }

    // Make the SPIR-V type
    Pspv::Id spvType = builder.makeStructType(spvMembers, type.getTypeName().c_str());
    if (! HasNonLayoutQualifiers(type, qualifier))
        structMap[explicitLayout][qualifier.layoutMatrix][glslangMembers] = spvType;

    // Decorate it
    decorateStructType(type, glslangMembers, explicitLayout, qualifier, spvType);

    for (int i = 0; i < (int)deferredForwardPointers.size(); ++i) {
        auto it = deferredForwardPointers[i];
        convertGlslangToSpvType(*it.first, explicitLayout, it.second, false);
    }

    return spvType;
}

void TGlslangToSpvTraverser::decorateStructType(const Pglslang::TType& type,
                                                const Pglslang::TTypeList* glslangMembers,
                                                Pglslang::TLayoutPacking explicitLayout,
                                                const Pglslang::TQualifier& qualifier,
                                                Pspv::Id spvType)
{
    // Name and decorate the non-hidden members
    int offset = -1;
    int locationOffset = 0;  // for use within the members of this struct
    for (int i = 0; i < (int)glslangMembers->size(); i++) {
        Pglslang::TType& glslangMember = *(*glslangMembers)[i].type;
        int member = i;
        if (type.getBasicType() == Pglslang::EbtBlock) {
            member = memberRemapper[glslangTypeToIdMap[glslangMembers]][i];
            if (filterMember(glslangMember))
                continue;
        }

        // modify just this child's view of the qualifier
        Pglslang::TQualifier memberQualifier = glslangMember.getQualifier();
        InheritQualifiers(memberQualifier, qualifier);

        // using -1 above to indicate a hidden member
        if (member < 0)
            continue;

        builder.addMemberName(spvType, member, glslangMember.getFieldName().c_str());
        builder.addMemberDecoration(spvType, member,
                                    TranslateLayoutDecoration(glslangMember, memberQualifier.layoutMatrix));
        builder.addMemberDecoration(spvType, member, TranslatePrecisionDecoration(glslangMember));
        // Add interpolation and auxiliary storage decorations only to
        // top-level members of Input and Output storage classes
        if (type.getQualifier().storage == Pglslang::EvqVaryingIn ||
            type.getQualifier().storage == Pglslang::EvqVaryingOut) {
            if (type.getBasicType() == Pglslang::EbtBlock ||
                glslangIntermediate->getSource() == Pglslang::EShSourceHlsl) {
                builder.addMemberDecoration(spvType, member, TranslateInterpolationDecoration(memberQualifier));
                builder.addMemberDecoration(spvType, member, TranslateAuxiliaryStorageDecoration(memberQualifier));
#ifndef GLSLANG_WEB
                addMeshNVDecoration(spvType, member, memberQualifier);
#endif
            }
        }
        builder.addMemberDecoration(spvType, member, TranslateInvariantDecoration(memberQualifier));

#ifndef GLSLANG_WEB
        if (type.getBasicType() == Pglslang::EbtBlock &&
            qualifier.storage == Pglslang::EvqBuffer) {
            // Add memory decorations only to top-level members of shader storage block
            std::vector<Pspv::Decoration> memory;
            TranslateMemoryDecoration(memberQualifier, memory, glslangIntermediate->usingVulkanMemoryModel());
            for (unsigned int i = 0; i < memory.size(); ++i)
                builder.addMemberDecoration(spvType, member, memory[i]);
        }

#endif

        // Location assignment was already completed correctly by the front end,
        // just track whether a member needs to be decorated.
        // Ignore member locations if the container is an array, as that's
        // ill-specified and decisions have been made to not allow this.
        if (! type.isArray() && memberQualifier.hasLocation())
            builder.addMemberDecoration(spvType, member, Pspv::DecorationLocation, memberQualifier.layoutLocation);

        if (qualifier.hasLocation())      // track for upcoming inheritance
            locationOffset += glslangIntermediate->PcomputeTypeLocationSize(
                                            glslangMember, glslangIntermediate->getStage());

        // component, XFB, others
        if (glslangMember.getQualifier().hasComponent())
            builder.addMemberDecoration(spvType, member, Pspv::DecorationComponent,
                                        glslangMember.getQualifier().layoutComponent);
        if (glslangMember.getQualifier().hasXfbOffset())
            builder.addMemberDecoration(spvType, member, Pspv::DecorationOffset,
                                        glslangMember.getQualifier().layoutXfbOffset);
        else if (explicitLayout != Pglslang::ElpNone) {
            // figure out what to do with offset, which is accumulating
            int nextOffset;
            updateMemberOffset(type, glslangMember, offset, nextOffset, explicitLayout, memberQualifier.layoutMatrix);
            if (offset >= 0)
                builder.addMemberDecoration(spvType, member, Pspv::DecorationOffset, offset);
            offset = nextOffset;
        }

        if (glslangMember.isMatrix() && explicitLayout != Pglslang::ElpNone)
            builder.addMemberDecoration(spvType, member, Pspv::DecorationMatrixStride,
                                        getMatrixStride(glslangMember, explicitLayout, memberQualifier.layoutMatrix));

        // built-in variable decorations
        Pspv::BuiltIn builtIn = TranslateBuiltInDecoration(glslangMember.getQualifier().builtIn, true);
        if (builtIn != Pspv::BuiltInMax)
            builder.addMemberDecoration(spvType, member, Pspv::DecorationBuiltIn, (int)builtIn);

#ifndef GLSLANG_WEB
        // nonuniform
        builder.addMemberDecoration(spvType, member, TranslateNonUniformDecoration(glslangMember.getQualifier()));

        if (glslangIntermediate->getHlslFunctionality1() && memberQualifier.semanticName != nullptr) {
            builder.addExtension("SPV_GOOGLE_hlsl_functionality1");
            builder.addMemberDecoration(spvType, member, (Pspv::Decoration)Pspv::DecorationHlslSemanticGOOGLE,
                                        memberQualifier.semanticName);
        }

        if (builtIn == Pspv::BuiltInLayer) {
            // SPV_NV_viewport_array2 extension
            if (glslangMember.getQualifier().layoutViewportRelative){
                builder.addMemberDecoration(spvType, member, (Pspv::Decoration)Pspv::DecorationViewportRelativeNV);
                builder.addCapability(Pspv::CapabilityShaderViewportMaskNV);
                builder.addExtension(Pspv::E_SPV_NV_viewport_array2);
            }
            if (glslangMember.getQualifier().layoutSecondaryViewportRelativeOffset != -2048){
                builder.addMemberDecoration(spvType, member,
                                            (Pspv::Decoration)Pspv::DecorationSecondaryViewportRelativeNV,
                                            glslangMember.getQualifier().layoutSecondaryViewportRelativeOffset);
                builder.addCapability(Pspv::CapabilityShaderStereoViewNV);
                builder.addExtension(Pspv::E_SPV_NV_stereo_view_rendering);
            }
        }
        if (glslangMember.getQualifier().layoutPassthrough) {
            builder.addMemberDecoration(spvType, member, (Pspv::Decoration)Pspv::DecorationPassthroughNV);
            builder.addCapability(Pspv::CapabilityGeometryShaderPassthroughNV);
            builder.addExtension(Pspv::E_SPV_NV_geometry_shader_passthrough);
        }
#endif
    }

    // Decorate the structure
    builder.addDecoration(spvType, TranslateLayoutDecoration(type, qualifier.layoutMatrix));
    builder.addDecoration(spvType, TranslateBlockDecoration(type, glslangIntermediate->usingStorageBuffer()));
}

// Turn the expression forming the array size into an id.
// This is not quite trivial, because of specialization constants.
// Sometimes, a raw constant is turned into an Id, and sometimes
// a specialization constant expression is.
Pspv::Id TGlslangToSpvTraverser::makeArraySizeId(const Pglslang::TArraySizes& arraySizes, int dim)
{
    // First, see if this is sized with a node, meaning a specialization constant:
    Pglslang::TIntermTyped* specNode = arraySizes.getDimNode(dim);
    if (specNode != nullptr) {
        builder.clearAccessChain();
        specNode->traverse(this);
        return accessChainLoad(specNode->getAsTyped()->getType());
    }

    // Otherwise, need a compile-time (front end) size, get it:
    int size = arraySizes.getDimSize(dim);
    assert(size > 0);
    return builder.makeUintConstant(size);
}

// Wrap the builder's accessChainLoad to:
//  - localize handling of RelaxedPrecision
//  - use the SPIR-V inferred type instead of another conversion of the glslang type
//    (avoids unnecessary work and possible type punning for structures)
//  - do conversion of concrete to abstract type
Pspv::Id TGlslangToSpvTraverser::accessChainLoad(const Pglslang::TType& type)
{
    Pspv::Id nominalTypeId = builder.accessChainGetInferredType();

    Pspv::Builder::AccessChain::CoherentFlags coherentFlags = builder.getAccessChain().coherentFlags;
    coherentFlags |= TranslateCoherent(type);

    unsigned int alignment = builder.getAccessChain().alignment;
    alignment |= type.getBufferReferenceAlignment();

    Pspv::Id loadedId = builder.accessChainLoad(TranslatePrecisionDecoration(type),
                                               TranslateNonUniformDecoration(type.getQualifier()),
                                               nominalTypeId,
                                               Pspv::MemoryAccessMask(TranslateMemoryAccess(coherentFlags) & ~Pspv::MemoryAccessMakePointerAvailableKHRMask),
                                               TranslateMemoryScope(coherentFlags),
                                               alignment);

    // Need to convert to abstract types when necessary
    if (type.getBasicType() == Pglslang::EbtBool) {
        if (builder.isScalarType(nominalTypeId)) {
            // Conversion for bool
            Pspv::Id boolType = builder.makeBoolType();
            if (nominalTypeId != boolType)
                loadedId = builder.createBinOp(Pspv::OpINotEqual, boolType, loadedId, builder.makeUintConstant(0));
        } else if (builder.isVectorType(nominalTypeId)) {
            // Conversion for bvec
            int vecSize = builder.getNumTypeComponents(nominalTypeId);
            Pspv::Id bvecType = builder.makeVectorType(builder.makeBoolType(), vecSize);
            if (nominalTypeId != bvecType)
                loadedId = builder.createBinOp(Pspv::OpINotEqual, bvecType, loadedId, makeSmearedConstant(builder.makeUintConstant(0), vecSize));
        }
    }

    return loadedId;
}

// Wrap the builder's accessChainStore to:
//  - do conversion of concrete to abstract type
//
// Implicitly uses the existing builder.accessChain as the storage target.
void TGlslangToSpvTraverser::accessChainStore(const Pglslang::TType& type, Pspv::Id rvalue)
{
    // Need to convert to abstract types when necessary
    if (type.getBasicType() == Pglslang::EbtBool) {
        Pspv::Id nominalTypeId = builder.accessChainGetInferredType();

        if (builder.isScalarType(nominalTypeId)) {
            // Conversion for bool
            Pspv::Id boolType = builder.makeBoolType();
            if (nominalTypeId != boolType) {
                // keep these outside arguments, for determinant order-of-evaluation
                Pspv::Id one = builder.makeUintConstant(1);
                Pspv::Id zero = builder.makeUintConstant(0);
                rvalue = builder.createTriOp(Pspv::OpSelect, nominalTypeId, rvalue, one, zero);
            } else if (builder.getTypeId(rvalue) != boolType)
                rvalue = builder.createBinOp(Pspv::OpINotEqual, boolType, rvalue, builder.makeUintConstant(0));
        } else if (builder.isVectorType(nominalTypeId)) {
            // Conversion for bvec
            int vecSize = builder.getNumTypeComponents(nominalTypeId);
            Pspv::Id bvecType = builder.makeVectorType(builder.makeBoolType(), vecSize);
            if (nominalTypeId != bvecType) {
                // keep these outside arguments, for determinant order-of-evaluation
                Pspv::Id one = makeSmearedConstant(builder.makeUintConstant(1), vecSize);
                Pspv::Id zero = makeSmearedConstant(builder.makeUintConstant(0), vecSize);
                rvalue = builder.createTriOp(Pspv::OpSelect, nominalTypeId, rvalue, one, zero);
            } else if (builder.getTypeId(rvalue) != bvecType)
                rvalue = builder.createBinOp(Pspv::OpINotEqual, bvecType, rvalue,
                                             makeSmearedConstant(builder.makeUintConstant(0), vecSize));
        }
    }

    Pspv::Builder::AccessChain::CoherentFlags coherentFlags = builder.getAccessChain().coherentFlags;
    coherentFlags |= TranslateCoherent(type);

    unsigned int alignment = builder.getAccessChain().alignment;
    alignment |= type.getBufferReferenceAlignment();

    builder.accessChainStore(rvalue,
                             Pspv::MemoryAccessMask(TranslateMemoryAccess(coherentFlags) & ~Pspv::MemoryAccessMakePointerVisibleKHRMask),
                             TranslateMemoryScope(coherentFlags), alignment);
}

// For storing when types match at the glslang level, but not might match at the
// SPIR-V level.
//
// This especially happens when a single glslang type expands to multiple
// SPIR-V types, like a struct that is used in a member-undecorated way as well
// as in a member-decorated way.
//
// NOTE: This function can handle any store request; if it's not special it
// simplifies to a simple OpStore.
//
// Implicitly uses the existing builder.accessChain as the storage target.
void TGlslangToSpvTraverser::multiTypeStore(const Pglslang::TType& type, Pspv::Id rValue)
{
    // we only do the complex path here if it's an aggregate
    if (! type.isStruct() && ! type.isArray()) {
        accessChainStore(type, rValue);
        return;
    }

    // and, it has to be a case of type aliasing
    Pspv::Id rType = builder.getTypeId(rValue);
    Pspv::Id lValue = builder.accessChainGetLValue();
    Pspv::Id lType = builder.getContainedTypeId(builder.getTypeId(lValue));
    if (lType == rType) {
        accessChainStore(type, rValue);
        return;
    }

    // Recursively (as needed) copy an aggregate type to a different aggregate type,
    // where the two types were the same type in GLSL. This requires member
    // by member copy, recursively.

    // SPIR-V 1.4 added an instruction to do help do this.
    if (glslangIntermediate->getSpv().spv >= Pglslang::EShTargetSpv_1_4) {
        // However, bool in uniform space is changed to int, so
        // OpCopyLogical does not work for that.
        // TODO: It would be more robust to do a full recursive verification of the types satisfying SPIR-V rules.
        bool rBool = builder.containsType(builder.getTypeId(rValue), Pspv::OpTypeBool, 0);
        bool lBool = builder.containsType(lType, Pspv::OpTypeBool, 0);
        if (lBool == rBool) {
            Pspv::Id logicalCopy = builder.createUnaryOp(Pspv::OpCopyLogical, lType, rValue);
            accessChainStore(type, logicalCopy);
            return;
        }
    }

    // If an array, copy element by element.
    if (type.isArray()) {
        Pglslang::TType glslangElementType(type, 0);
        Pspv::Id elementRType = builder.getContainedTypeId(rType);
        for (int index = 0; index < type.getOuterArraySize(); ++index) {
            // get the source member
            Pspv::Id elementRValue = builder.createCompositeExtract(rValue, elementRType, index);

            // set up the target storage
            builder.clearAccessChain();
            builder.setAccessChainLValue(lValue);
            builder.accessChainPush(builder.makeIntConstant(index), TranslateCoherent(type), type.getBufferReferenceAlignment());

            // store the member
            multiTypeStore(glslangElementType, elementRValue);
        }
    } else {
        assert(type.isStruct());

        // loop over structure members
        const Pglslang::TTypeList& members = *type.getStruct();
        for (int m = 0; m < (int)members.size(); ++m) {
            const Pglslang::TType& glslangMemberType = *members[m].type;

            // get the source member
            Pspv::Id memberRType = builder.getContainedTypeId(rType, m);
            Pspv::Id memberRValue = builder.createCompositeExtract(rValue, memberRType, m);

            // set up the target storage
            builder.clearAccessChain();
            builder.setAccessChainLValue(lValue);
            builder.accessChainPush(builder.makeIntConstant(m), TranslateCoherent(type), type.getBufferReferenceAlignment());

            // store the member
            multiTypeStore(glslangMemberType, memberRValue);
        }
    }
}

// Decide whether or not this type should be
// decorated with offsets and strides, and if so
// whether std140 or std430 rules should be applied.
Pglslang::TLayoutPacking TGlslangToSpvTraverser::getExplicitLayout(const Pglslang::TType& type) const
{
    // has to be a block
    if (type.getBasicType() != Pglslang::EbtBlock)
        return Pglslang::ElpNone;

    // has to be a uniform or buffer block or task in/out blocks
    if (type.getQualifier().storage != Pglslang::EvqUniform &&
        type.getQualifier().storage != Pglslang::EvqBuffer &&
        !type.getQualifier().isTaskMemory())
        return Pglslang::ElpNone;

    // return the layout to use
    switch (type.getQualifier().layoutPacking) {
    case Pglslang::ElpStd140:
    case Pglslang::ElpStd430:
    case Pglslang::ElpScalar:
        return type.getQualifier().layoutPacking;
    default:
        return Pglslang::ElpNone;
    }
}

// Given an array type, returns the integer stride required for that array
int TGlslangToSpvTraverser::getArrayStride(const Pglslang::TType& arrayType, Pglslang::TLayoutPacking explicitLayout, Pglslang::TLayoutMatrix matrixLayout)
{
    int size;
    int stride;
    glslangIntermediate->getMemberAlignment(arrayType, size, stride, explicitLayout, matrixLayout == Pglslang::ElmRowMajor);

    return stride;
}

// Given a matrix type, or array (of array) of matrixes type, returns the integer stride required for that matrix
// when used as a member of an interface block
int TGlslangToSpvTraverser::getMatrixStride(const Pglslang::TType& matrixType, Pglslang::TLayoutPacking explicitLayout, Pglslang::TLayoutMatrix matrixLayout)
{
    Pglslang::TType elementType;
    elementType.shallowCopy(matrixType);
    elementType.clearArraySizes();

    int size;
    int stride;
    glslangIntermediate->getMemberAlignment(elementType, size, stride, explicitLayout, matrixLayout == Pglslang::ElmRowMajor);

    return stride;
}

// Given a member type of a struct, realign the current offset for it, and compute
// the next (not yet aligned) offset for the next member, which will get aligned
// on the next call.
// 'currentOffset' should be passed in already initialized, ready to modify, and reflecting
// the migration of data from nextOffset -> currentOffset.  It should be -1 on the first call.
// -1 means a non-forced member offset (no decoration needed).
void TGlslangToSpvTraverser::updateMemberOffset(const Pglslang::TType& structType, const Pglslang::TType& memberType, int& currentOffset, int& nextOffset,
                                                Pglslang::TLayoutPacking explicitLayout, Pglslang::TLayoutMatrix matrixLayout)
{
    // this will get a positive value when deemed necessary
    nextOffset = -1;

    // override anything in currentOffset with user-set offset
    if (memberType.getQualifier().hasOffset())
        currentOffset = memberType.getQualifier().layoutOffset;

    // It could be that current linker usage in glslang updated all the layoutOffset,
    // in which case the following code does not matter.  But, that's not quite right
    // once cross-compilation unit GLSL validation is done, as the original user
    // settings are needed in layoutOffset, and then the following will come into play.

    if (explicitLayout == Pglslang::ElpNone) {
        if (! memberType.getQualifier().hasOffset())
            currentOffset = -1;

        return;
    }

    // Getting this far means we need explicit offsets
    if (currentOffset < 0)
        currentOffset = 0;

    // Now, currentOffset is valid (either 0, or from a previous nextOffset),
    // but possibly not yet correctly aligned.

    int memberSize;
    int dummyStride;
    int memberAlignment = glslangIntermediate->getMemberAlignment(memberType, memberSize, dummyStride, explicitLayout, matrixLayout == Pglslang::ElmRowMajor);

    // Adjust alignment for HLSL rules
    // TODO: make this consistent in early phases of code:
    //       adjusting this late means inconsistencies with earlier code, which for reflection is an issue
    // Until reflection is brought in sync with these adjustments, don't apply to $Global,
    // which is the most likely to rely on reflection, and least likely to rely implicit layouts
    if (glslangIntermediate->usingHlslOffsets() &&
        ! memberType.isArray() && memberType.isVector() && structType.getTypeName().compare("$Global") != 0) {
        int dummySize;
        int componentAlignment = glslangIntermediate->PgetBaseAlignmentScalar(memberType, dummySize);
        if (componentAlignment <= 4)
            memberAlignment = componentAlignment;
    }

    // Bump up to member alignment
    Pglslang::RoundToPow2(currentOffset, memberAlignment);

    // Bump up to vec4 if there is a bad straddle
    if (explicitLayout != Pglslang::ElpScalar && glslangIntermediate->PimproperStraddle(memberType, memberSize, currentOffset))
        Pglslang::RoundToPow2(currentOffset, 16);

    nextOffset = currentOffset + memberSize;
}

void TGlslangToSpvTraverser::declareUseOfStructMember(const Pglslang::TTypeList& members, int glslangMember)
{
    const Pglslang::TBuiltInVariable glslangBuiltIn = members[glslangMember].type->getQualifier().builtIn;
    switch (glslangBuiltIn)
    {
    case Pglslang::EbvPointSize:
#ifndef GLSLANG_WEB
    case Pglslang::EbvClipDistance:
    case Pglslang::EbvCullDistance:
    case Pglslang::EbvViewportMaskNV:
    case Pglslang::EbvSecondaryPositionNV:
    case Pglslang::EbvSecondaryViewportMaskNV:
    case Pglslang::EbvPositionPerViewNV:
    case Pglslang::EbvViewportMaskPerViewNV:
    case Pglslang::EbvTaskCountNV:
    case Pglslang::EbvPrimitiveCountNV:
    case Pglslang::EbvPrimitiveIndicesNV:
    case Pglslang::EbvClipDistancePerViewNV:
    case Pglslang::EbvCullDistancePerViewNV:
    case Pglslang::EbvLayerPerViewNV:
    case Pglslang::EbvMeshViewCountNV:
    case Pglslang::EbvMeshViewIndicesNV:
#endif
        // Generate the associated capability.  Delegate to TranslateBuiltInDecoration.
        // Alternately, we could just call this for any glslang built-in, since the
        // capability already guards against duplicates.
        TranslateBuiltInDecoration(glslangBuiltIn, false);
        break;
    default:
        // Capabilities were already generated when the struct was declared.
        break;
    }
}

bool TGlslangToSpvTraverser::isShaderEntryPoint(const Pglslang::TIntermAggregate* node)
{
    return node->getName().compare(glslangIntermediate->getEntryPointMangledName().c_str()) == 0;
}

// Does parameter need a place to keep writes, separate from the original?
// Assumes called after originalParam(), which filters out block/buffer/opaque-based
// qualifiers such that we should have only in/out/inout/constreadonly here.
bool TGlslangToSpvTraverser::writableParam(Pglslang::TStorageQualifier qualifier) const
{
    assert(qualifier == Pglslang::EvqIn ||
           qualifier == Pglslang::EvqOut ||
           qualifier == Pglslang::EvqInOut ||
           qualifier == Pglslang::EvqConstReadOnly);
    return qualifier != Pglslang::EvqConstReadOnly;
}

// Is parameter pass-by-original?
bool TGlslangToSpvTraverser::originalParam(Pglslang::TStorageQualifier qualifier, const Pglslang::TType& paramType,
                                           bool implicitThisParam)
{
    if (implicitThisParam)                                                                     // implicit this
        return true;
    if (glslangIntermediate->getSource() == Pglslang::EShSourceHlsl)
        return paramType.getBasicType() == Pglslang::EbtBlock;
    return paramType.containsOpaque() ||                                                       // sampler, etc.
           (paramType.getBasicType() == Pglslang::EbtBlock && qualifier == Pglslang::EvqBuffer); // SSBO
}

// Make all the functions, skeletally, without actually visiting their bodies.
void TGlslangToSpvTraverser::makeFunctions(const Pglslang::TIntermSequence& glslFunctions)
{
    const auto getParamDecorations = [&](std::vector<Pspv::Decoration>& decorations, const Pglslang::TType& type, bool useVulkanMemoryModel) {
        Pspv::Decoration paramPrecision = TranslatePrecisionDecoration(type);
        if (paramPrecision != Pspv::NoPrecision)
            decorations.push_back(paramPrecision);
        TranslateMemoryDecoration(type.getQualifier(), decorations, useVulkanMemoryModel);
        if (type.isReference()) {
            // Original and non-writable params pass the pointer directly and
            // use restrict/aliased, others are stored to a pointer in Function
            // memory and use RestrictPointer/AliasedPointer.
            if (originalParam(type.getQualifier().storage, type, false) ||
                !writableParam(type.getQualifier().storage)) {
                decorations.push_back(type.getQualifier().isRestrict() ? Pspv::DecorationRestrict :
                                                                         Pspv::DecorationAliased);
            } else {
                decorations.push_back(type.getQualifier().isRestrict() ? Pspv::DecorationRestrictPointerEXT :
                                                                         Pspv::DecorationAliasedPointerEXT);
            }
        }
    };

    for (int f = 0; f < (int)glslFunctions.size(); ++f) {
        Pglslang::TIntermAggregate* glslFunction = glslFunctions[f]->getAsAggregate();
        if (! glslFunction || glslFunction->getOp() != Pglslang::EOpFunction || isShaderEntryPoint(glslFunction))
            continue;

        // We're on a user function.  Set up the basic interface for the function now,
        // so that it's available to call.  Translating the body will happen later.
        //
        // Typically (except for a "const in" parameter), an address will be passed to the
        // function.  What it is an address of varies:
        //
        // - "in" parameters not marked as "const" can be written to without modifying the calling
        //   argument so that write needs to be to a copy, hence the address of a copy works.
        //
        // - "const in" parameters can just be the r-value, as no writes need occur.
        //
        // - "out" and "inout" arguments can't be done as pointers to the calling argument, because
        //   GLSL has copy-in/copy-out semantics.  They can be handled though with a pointer to a copy.

        std::vector<Pspv::Id> paramTypes;
        std::vector<std::vector<Pspv::Decoration>> paramDecorations; // list of decorations per parameter
        Pglslang::TIntermSequence& parameters = glslFunction->getSequence()[0]->getAsAggregate()->getSequence();

#ifdef ENABLE_HLSL
        bool implicitThis = (int)parameters.size() > 0 && parameters[0]->getAsSymbolNode()->getName() ==
                                                          glslangIntermediate->implicitThisName;
#else
        bool implicitThis = false;
#endif

        paramDecorations.resize(parameters.size());
        for (int p = 0; p < (int)parameters.size(); ++p) {
            const Pglslang::TType& paramType = parameters[p]->getAsTyped()->getType();
            Pspv::Id typeId = convertGlslangToSpvType(paramType);
            if (originalParam(paramType.getQualifier().storage, paramType, implicitThis && p == 0))
                typeId = builder.makePointer(TranslateStorageClass(paramType), typeId);
            else if (writableParam(paramType.getQualifier().storage))
                typeId = builder.makePointer(Pspv::StorageClassFunction, typeId);
            else
                rValueParameters.insert(parameters[p]->getAsSymbolNode()->getId());
            getParamDecorations(paramDecorations[p], paramType, glslangIntermediate->usingVulkanMemoryModel());
            paramTypes.push_back(typeId);
        }

        Pspv::Block* functionBlock;
        Pspv::Function *function = builder.makeFunctionEntry(TranslatePrecisionDecoration(glslFunction->getType()),
                                                            convertGlslangToSpvType(glslFunction->getType()),
                                                            glslFunction->getName().c_str(), paramTypes,
                                                            paramDecorations, &functionBlock);
        if (implicitThis)
            function->setImplicitThis();

        // Track function to emit/call later
        functionMap[glslFunction->getName().c_str()] = function;

        // Set the parameter id's
        for (int p = 0; p < (int)parameters.size(); ++p) {
            symbolValues[parameters[p]->getAsSymbolNode()->getId()] = function->getParamId(p);
            // give a name too
            builder.addName(function->getParamId(p), parameters[p]->getAsSymbolNode()->getName().c_str());

            const Pglslang::TType& paramType = parameters[p]->getAsTyped()->getType();
            if (paramType.contains8BitInt())
                builder.addCapability(Pspv::CapabilityInt8);
            if (paramType.contains16BitInt())
                builder.addCapability(Pspv::CapabilityInt16);
            if (paramType.contains16BitFloat())
                builder.addCapability(Pspv::CapabilityFloat16);
        }
    }
}

// Process all the initializers, while skipping the functions and link objects
void TGlslangToSpvTraverser::makeGlobalInitializers(const Pglslang::TIntermSequence& initializers)
{
    builder.setBuildPoint(shaderEntry->getLastBlock());
    for (int i = 0; i < (int)initializers.size(); ++i) {
        Pglslang::TIntermAggregate* initializer = initializers[i]->getAsAggregate();
        if (initializer && initializer->getOp() != Pglslang::EOpFunction && initializer->getOp() != Pglslang::EOpLinkerObjects) {

            // We're on a top-level node that's not a function.  Treat as an initializer, whose
            // code goes into the beginning of the entry point.
            initializer->traverse(this);
        }
    }
}

// Process all the functions, while skipping initializers.
void TGlslangToSpvTraverser::visitFunctions(const Pglslang::TIntermSequence& glslFunctions)
{
    for (int f = 0; f < (int)glslFunctions.size(); ++f) {
        Pglslang::TIntermAggregate* node = glslFunctions[f]->getAsAggregate();
        if (node && (node->getOp() == Pglslang::EOpFunction || node->getOp() == Pglslang::EOpLinkerObjects))
            node->traverse(this);
    }
}

void TGlslangToSpvTraverser::handleFunctionEntry(const Pglslang::TIntermAggregate* node)
{
    // SPIR-V functions should already be in the functionMap from the prepass
    // that called makeFunctions().
    currentFunction = functionMap[node->getName().c_str()];
    Pspv::Block* functionBlock = currentFunction->getEntryBlock();
    builder.setBuildPoint(functionBlock);
}

void TGlslangToSpvTraverser::translateArguments(const Pglslang::TIntermAggregate& node, std::vector<Pspv::Id>& arguments, Pspv::Builder::AccessChain::CoherentFlags &lvalueCoherentFlags)
{
    const Pglslang::TIntermSequence& glslangArguments = node.getSequence();

    Pglslang::TSampler sampler = {};
    bool cubeCompare = false;
#ifndef GLSLANG_WEB
    bool f16ShadowCompare = false;
#endif
    if (node.isTexture() || node.isImage()) {
        sampler = glslangArguments[0]->getAsTyped()->getType().getSampler();
        cubeCompare = sampler.dim == Pglslang::EsdCube && sampler.arrayed && sampler.shadow;
#ifndef GLSLANG_WEB
        f16ShadowCompare = sampler.shadow && glslangArguments[1]->getAsTyped()->getType().getBasicType() == Pglslang::EbtFloat16;
#endif
    }

    for (int i = 0; i < (int)glslangArguments.size(); ++i) {
        builder.clearAccessChain();
        glslangArguments[i]->traverse(this);

#ifndef GLSLANG_WEB
        // Special case l-value operands
        bool lvalue = false;
        switch (node.getOp()) {
        case Pglslang::EOpImageAtomicAdd:
        case Pglslang::EOpImageAtomicMin:
        case Pglslang::EOpImageAtomicMax:
        case Pglslang::EOpImageAtomicAnd:
        case Pglslang::EOpImageAtomicOr:
        case Pglslang::EOpImageAtomicXor:
        case Pglslang::EOpImageAtomicExchange:
        case Pglslang::EOpImageAtomicCompSwap:
        case Pglslang::EOpImageAtomicLoad:
        case Pglslang::EOpImageAtomicStore:
            if (i == 0)
                lvalue = true;
            break;
        case Pglslang::EOpSparseImageLoad:
            if ((sampler.ms && i == 3) || (! sampler.ms && i == 2))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTexture:
            if (((cubeCompare || f16ShadowCompare) && i == 3) || (! (cubeCompare || f16ShadowCompare) && i == 2))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureClamp:
            if (((cubeCompare || f16ShadowCompare) && i == 4) || (! (cubeCompare || f16ShadowCompare) && i == 3))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureLod:
        case Pglslang::EOpSparseTextureOffset:
            if  ((f16ShadowCompare && i == 4) || (! f16ShadowCompare && i == 3))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureFetch:
            if ((sampler.dim != Pglslang::EsdRect && i == 3) || (sampler.dim == Pglslang::EsdRect && i == 2))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureFetchOffset:
            if ((sampler.dim != Pglslang::EsdRect && i == 4) || (sampler.dim == Pglslang::EsdRect && i == 3))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureLodOffset:
        case Pglslang::EOpSparseTextureGrad:
        case Pglslang::EOpSparseTextureOffsetClamp:
            if ((f16ShadowCompare && i == 5) || (! f16ShadowCompare && i == 4))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureGradOffset:
        case Pglslang::EOpSparseTextureGradClamp:
            if ((f16ShadowCompare && i == 6) || (! f16ShadowCompare && i == 5))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureGradOffsetClamp:
            if ((f16ShadowCompare && i == 7) || (! f16ShadowCompare && i == 6))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureGather:
            if ((sampler.shadow && i == 3) || (! sampler.shadow && i == 2))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureGatherOffset:
        case Pglslang::EOpSparseTextureGatherOffsets:
            if ((sampler.shadow && i == 4) || (! sampler.shadow && i == 3))
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureGatherLod:
            if (i == 3)
                lvalue = true;
            break;
        case Pglslang::EOpSparseTextureGatherLodOffset:
        case Pglslang::EOpSparseTextureGatherLodOffsets:
            if (i == 4)
                lvalue = true;
            break;
        case Pglslang::EOpSparseImageLoadLod:
            if (i == 3)
                lvalue = true;
            break;
        case Pglslang::EOpImageSampleFootprintNV:
            if (i == 4)
                lvalue = true;
            break;
        case Pglslang::EOpImageSampleFootprintClampNV:
        case Pglslang::EOpImageSampleFootprintLodNV:
            if (i == 5)
                lvalue = true;
            break;
        case Pglslang::EOpImageSampleFootprintGradNV:
            if (i == 6)
                lvalue = true;
            break;
        case Pglslang::EOpImageSampleFootprintGradClampNV:
            if (i == 7)
                lvalue = true;
            break;
        default:
            break;
        }

        if (lvalue) {
            arguments.push_back(builder.accessChainGetLValue());
            lvalueCoherentFlags = builder.getAccessChain().coherentFlags;
            lvalueCoherentFlags |= TranslateCoherent(glslangArguments[i]->getAsTyped()->getType());
        } else
#endif
            arguments.push_back(accessChainLoad(glslangArguments[i]->getAsTyped()->getType()));
    }
}

void TGlslangToSpvTraverser::translateArguments(Pglslang::TIntermUnary& node, std::vector<Pspv::Id>& arguments)
{
    builder.clearAccessChain();
    node.getOperand()->traverse(this);
    arguments.push_back(accessChainLoad(node.getOperand()->getType()));
}

Pspv::Id TGlslangToSpvTraverser::createImageTextureFunctionCall(Pglslang::TIntermOperator* node)
{
    if (! node->isImage() && ! node->isTexture())
        return Pspv::NoResult;

    builder.setLine(node->getLoc().line, node->getLoc().getFilename());

    // Process a GLSL texturing op (will be SPV image)

    const Pglslang::TType &imageType = node->getAsAggregate()
                                        ? node->getAsAggregate()->getSequence()[0]->getAsTyped()->getType()
                                        : node->getAsUnaryNode()->getOperand()->getAsTyped()->getType();
    const Pglslang::TSampler sampler = imageType.getSampler();
#ifdef GLSLANG_WEB
    const bool f16ShadowCompare = false;
#else
    bool f16ShadowCompare = (sampler.shadow && node->getAsAggregate())
            ? node->getAsAggregate()->getSequence()[1]->getAsTyped()->getType().getBasicType() == Pglslang::EbtFloat16
            : false;
#endif

    const auto signExtensionMask = [&]() {
        if (builder.getSpvVersion() >= Pspv::Spv_1_4) {
            if (sampler.type == Pglslang::EbtUint)
                return Pspv::ImageOperandsZeroExtendMask;
            else if (sampler.type == Pglslang::EbtInt)
                return Pspv::ImageOperandsSignExtendMask;
        }
        return Pspv::ImageOperandsMaskNone;
    };

    Pspv::Builder::AccessChain::CoherentFlags lvalueCoherentFlags;

    std::vector<Pspv::Id> arguments;
    if (node->getAsAggregate())
        translateArguments(*node->getAsAggregate(), arguments, lvalueCoherentFlags);
    else
        translateArguments(*node->getAsUnaryNode(), arguments);
    Pspv::Decoration precision = TranslatePrecisionDecoration(node->getOperationPrecision());

    Pspv::Builder::TextureParameters params = { };
    params.sampler = arguments[0];

    Pglslang::TCrackedTextureOp cracked;
    node->crackTexture(sampler, cracked);

    const bool isUnsignedResult = node->getType().getBasicType() == Pglslang::EbtUint;

    // Check for queries
    if (cracked.query) {
        // OpImageQueryLod works on a sampled image, for other queries the image has to be extracted first
        if (node->getOp() != Pglslang::EOpTextureQueryLod && builder.isSampledImage(params.sampler))
            params.sampler = builder.createUnaryOp(Pspv::OpImage, builder.getImageType(params.sampler), params.sampler);

        switch (node->getOp()) {
        case Pglslang::EOpImageQuerySize:
        case Pglslang::EOpTextureQuerySize:
            if (arguments.size() > 1) {
                params.lod = arguments[1];
                return builder.createTextureQueryCall(Pspv::OpImageQuerySizeLod, params, isUnsignedResult);
            } else
                return builder.createTextureQueryCall(Pspv::OpImageQuerySize, params, isUnsignedResult);
#ifndef GLSLANG_WEB
        case Pglslang::EOpImageQuerySamples:
        case Pglslang::EOpTextureQuerySamples:
            return builder.createTextureQueryCall(Pspv::OpImageQuerySamples, params, isUnsignedResult);
        case Pglslang::EOpTextureQueryLod:
            params.coords = arguments[1];
            return builder.createTextureQueryCall(Pspv::OpImageQueryLod, params, isUnsignedResult);
        case Pglslang::EOpTextureQueryLevels:
            return builder.createTextureQueryCall(Pspv::OpImageQueryLevels, params, isUnsignedResult);
        case Pglslang::EOpSparseTexelsResident:
            return builder.createUnaryOp(Pspv::OpImageSparseTexelsResident, builder.makeBoolType(), arguments[0]);
#endif
        default:
            assert(0);
            break;
        }
    }

    int components = node->getType().getVectorSize();

    if (node->getOp() == Pglslang::EOpTextureFetch) {
        // These must produce 4 components, per SPIR-V spec.  We'll add a conversion constructor if needed.
        // This will only happen through the HLSL path for operator[], so we do not have to handle e.g.
        // the EOpTexture/Proj/Lod/etc family.  It would be harmless to do so, but would need more logic
        // here around e.g. which ones return scalars or other types.
        components = 4;
    }

    Pglslang::TType returnType(node->getType().getBasicType(), Pglslang::EvqTemporary, components);

    auto resultType = [&returnType,this]{ return convertGlslangToSpvType(returnType); };

    // Check for image functions other than queries
    if (node->isImage()) {
        std::vector<Pspv::IdImmediate> operands;
        auto opIt = arguments.begin();
        Pspv::IdImmediate image = { true, *(opIt++) };
        operands.push_back(image);

        // Handle subpass operations
        // TODO: GLSL should change to have the "MS" only on the type rather than the
        // built-in function.
        if (cracked.subpass) {
            // add on the (0,0) coordinate
            Pspv::Id zero = builder.makeIntConstant(0);
            std::vector<Pspv::Id> comps;
            comps.push_back(zero);
            comps.push_back(zero);
            Pspv::IdImmediate coord = { true,
                builder.makeCompositeConstant(builder.makeVectorType(builder.makeIntType(32), 2), comps) };
            operands.push_back(coord);
            Pspv::IdImmediate imageOperands = { false, Pspv::ImageOperandsMaskNone };
            imageOperands.word = imageOperands.word | signExtensionMask();
            if (sampler.isMultiSample()) {
                imageOperands.word = imageOperands.word | Pspv::ImageOperandsSampleMask;
            }
            if (imageOperands.word != Pspv::ImageOperandsMaskNone) {
                operands.push_back(imageOperands);
                if (sampler.isMultiSample()) {
                    Pspv::IdImmediate imageOperand = { true, *(opIt++) };
                    operands.push_back(imageOperand);
                }
            }
            Pspv::Id result = builder.createOp(Pspv::OpImageRead, resultType(), operands);
            builder.setPrecision(result, precision);
            return result;
        }

        Pspv::IdImmediate coord = { true, *(opIt++) };
        operands.push_back(coord);
        if (node->getOp() == Pglslang::EOpImageLoad || node->getOp() == Pglslang::EOpImageLoadLod) {
            Pspv::ImageOperandsMask mask = Pspv::ImageOperandsMaskNone;
            if (sampler.isMultiSample()) {
                mask = mask | Pspv::ImageOperandsSampleMask;
            }
            if (cracked.lod) {
                builder.addExtension(Pspv::E_SPV_AMD_shader_image_load_store_lod);
                builder.addCapability(Pspv::CapabilityImageReadWriteLodAMD);
                mask = mask | Pspv::ImageOperandsLodMask;
            }
            mask = mask | TranslateImageOperands(TranslateCoherent(imageType));
            mask = (Pspv::ImageOperandsMask)(mask & ~Pspv::ImageOperandsMakeTexelAvailableKHRMask);
            mask = mask | signExtensionMask();
            if (mask != Pspv::ImageOperandsMaskNone) {
                Pspv::IdImmediate imageOperands = { false, (unsigned int)mask };
                operands.push_back(imageOperands);
            }
            if (mask & Pspv::ImageOperandsSampleMask) {
                Pspv::IdImmediate imageOperand = { true, *opIt++ };
                operands.push_back(imageOperand);
            }
            if (mask & Pspv::ImageOperandsLodMask) {
                Pspv::IdImmediate imageOperand = { true, *opIt++ };
                operands.push_back(imageOperand);
            }
            if (mask & Pspv::ImageOperandsMakeTexelVisibleKHRMask) {
                Pspv::IdImmediate imageOperand = { true,
                                    builder.makeUintConstant(TranslateMemoryScope(TranslateCoherent(imageType))) };
                operands.push_back(imageOperand);
            }

            if (builder.getImageTypeFormat(builder.getImageType(operands.front().word)) == Pspv::ImageFormatUnknown)
                builder.addCapability(Pspv::CapabilityStorageImageReadWithoutFormat);

            std::vector<Pspv::Id> result(1, builder.createOp(Pspv::OpImageRead, resultType(), operands));
            builder.setPrecision(result[0], precision);

            // If needed, add a conversion constructor to the proper size.
            if (components != node->getType().getVectorSize())
                result[0] = builder.createConstructor(precision, result, convertGlslangToSpvType(node->getType()));

            return result[0];
        } else if (node->getOp() == Pglslang::EOpImageStore || node->getOp() == Pglslang::EOpImageStoreLod) {

            // Push the texel value before the operands
            if (sampler.isMultiSample() || cracked.lod) {
                Pspv::IdImmediate texel = { true, *(opIt + 1) };
                operands.push_back(texel);
            } else {
                Pspv::IdImmediate texel = { true, *opIt };
                operands.push_back(texel);
            }

            Pspv::ImageOperandsMask mask = Pspv::ImageOperandsMaskNone;
            if (sampler.isMultiSample()) {
                mask = mask | Pspv::ImageOperandsSampleMask;
            }
            if (cracked.lod) {
                builder.addExtension(Pspv::E_SPV_AMD_shader_image_load_store_lod);
                builder.addCapability(Pspv::CapabilityImageReadWriteLodAMD);
                mask = mask | Pspv::ImageOperandsLodMask;
            }
            mask = mask | TranslateImageOperands(TranslateCoherent(imageType));
            mask = (Pspv::ImageOperandsMask)(mask & ~Pspv::ImageOperandsMakeTexelVisibleKHRMask);
            mask = mask | signExtensionMask();
            if (mask != Pspv::ImageOperandsMaskNone) {
                Pspv::IdImmediate imageOperands = { false, (unsigned int)mask };
                operands.push_back(imageOperands);
            }
            if (mask & Pspv::ImageOperandsSampleMask) {
                Pspv::IdImmediate imageOperand = { true, *opIt++ };
                operands.push_back(imageOperand);
            }
            if (mask & Pspv::ImageOperandsLodMask) {
                Pspv::IdImmediate imageOperand = { true, *opIt++ };
                operands.push_back(imageOperand);
            }
            if (mask & Pspv::ImageOperandsMakeTexelAvailableKHRMask) {
                Pspv::IdImmediate imageOperand = { true,
                    builder.makeUintConstant(TranslateMemoryScope(TranslateCoherent(imageType))) };
                operands.push_back(imageOperand);
            }

            builder.createNoResultOp(Pspv::OpImageWrite, operands);
            if (builder.getImageTypeFormat(builder.getImageType(operands.front().word)) == Pspv::ImageFormatUnknown)
                builder.addCapability(Pspv::CapabilityStorageImageWriteWithoutFormat);
            return Pspv::NoResult;
        } else if (node->getOp() == Pglslang::EOpSparseImageLoad ||
                   node->getOp() == Pglslang::EOpSparseImageLoadLod) {
            builder.addCapability(Pspv::CapabilitySparseResidency);
            if (builder.getImageTypeFormat(builder.getImageType(operands.front().word)) == Pspv::ImageFormatUnknown)
                builder.addCapability(Pspv::CapabilityStorageImageReadWithoutFormat);

            Pspv::ImageOperandsMask mask = Pspv::ImageOperandsMaskNone;
            if (sampler.isMultiSample()) {
                mask = mask | Pspv::ImageOperandsSampleMask;
            }
            if (cracked.lod) {
                builder.addExtension(Pspv::E_SPV_AMD_shader_image_load_store_lod);
                builder.addCapability(Pspv::CapabilityImageReadWriteLodAMD);

                mask = mask | Pspv::ImageOperandsLodMask;
            }
            mask = mask | TranslateImageOperands(TranslateCoherent(imageType));
            mask = (Pspv::ImageOperandsMask)(mask & ~Pspv::ImageOperandsMakeTexelAvailableKHRMask);
            mask = mask | signExtensionMask();
            if (mask != Pspv::ImageOperandsMaskNone) {
                Pspv::IdImmediate imageOperands = { false, (unsigned int)mask };
                operands.push_back(imageOperands);
            }
            if (mask & Pspv::ImageOperandsSampleMask) {
                Pspv::IdImmediate imageOperand = { true, *opIt++ };
                operands.push_back(imageOperand);
            }
            if (mask & Pspv::ImageOperandsLodMask) {
                Pspv::IdImmediate imageOperand = { true, *opIt++ };
                operands.push_back(imageOperand);
            }
            if (mask & Pspv::ImageOperandsMakeTexelVisibleKHRMask) {
                Pspv::IdImmediate imageOperand = { true, builder.makeUintConstant(TranslateMemoryScope(TranslateCoherent(imageType))) };
                operands.push_back(imageOperand);
            }

            // Create the return type that was a special structure
            Pspv::Id texelOut = *opIt;
            Pspv::Id typeId0 = resultType();
            Pspv::Id typeId1 = builder.getDerefTypeId(texelOut);
            Pspv::Id resultTypeId = builder.makeStructResultType(typeId0, typeId1);

            Pspv::Id resultId = builder.createOp(Pspv::OpImageSparseRead, resultTypeId, operands);

            // Decode the return type
            builder.createStore(builder.createCompositeExtract(resultId, typeId1, 1), texelOut);
            return builder.createCompositeExtract(resultId, typeId0, 0);
        } else {
            // Process image atomic operations

            // GLSL "IMAGE_PARAMS" will involve in constructing an image texel pointer and this pointer,
            // as the first source operand, is required by SPIR-V atomic operations.
            // For non-MS, the sample value should be 0
            Pspv::IdImmediate sample = { true, sampler.isMultiSample() ? *(opIt++) : builder.makeUintConstant(0) };
            operands.push_back(sample);

            Pspv::Id resultTypeId;
            // imageAtomicStore has a void return type so base the pointer type on
            // the type of the value operand.
            if (node->getOp() == Pglslang::EOpImageAtomicStore) {
                resultTypeId = builder.makePointer(Pspv::StorageClassImage, builder.getTypeId(*opIt));
            } else {
                resultTypeId = builder.makePointer(Pspv::StorageClassImage, resultType());
            }
            Pspv::Id pointer = builder.createOp(Pspv::OpImageTexelPointer, resultTypeId, operands);

            std::vector<Pspv::Id> operands;
            operands.push_back(pointer);
            for (; opIt != arguments.end(); ++opIt)
                operands.push_back(*opIt);

            return createAtomicOperation(node->getOp(), precision, resultType(), operands, node->getBasicType(), lvalueCoherentFlags);
        }
    }

#ifndef GLSLANG_WEB
    // Check for fragment mask functions other than queries
    if (cracked.fragMask) {
        assert(sampler.ms);

        auto opIt = arguments.begin();
        std::vector<Pspv::Id> operands;

        // Extract the image if necessary
        if (builder.isSampledImage(params.sampler))
            params.sampler = builder.createUnaryOp(Pspv::OpImage, builder.getImageType(params.sampler), params.sampler);

        operands.push_back(params.sampler);
        ++opIt;

        if (sampler.isSubpass()) {
            // add on the (0,0) coordinate
            Pspv::Id zero = builder.makeIntConstant(0);
            std::vector<Pspv::Id> comps;
            comps.push_back(zero);
            comps.push_back(zero);
            operands.push_back(builder.makeCompositeConstant(builder.makeVectorType(builder.makeIntType(32), 2), comps));
        }

        for (; opIt != arguments.end(); ++opIt)
            operands.push_back(*opIt);

        Pspv::Op fragMaskOp = Pspv::OpNop;
        if (node->getOp() == Pglslang::EOpFragmentMaskFetch)
            fragMaskOp = Pspv::OpFragmentMaskFetchAMD;
        else if (node->getOp() == Pglslang::EOpFragmentFetch)
            fragMaskOp = Pspv::OpFragmentFetchAMD;

        builder.addExtension(Pspv::E_SPV_AMD_shader_fragment_mask);
        builder.addCapability(Pspv::CapabilityFragmentMaskAMD);
        return builder.createOp(fragMaskOp, resultType(), operands);
    }
#endif

    // Check for texture functions other than queries
    bool sparse = node->isSparseTexture();
    bool imageFootprint = node->isImageFootprint();
    bool cubeCompare = sampler.dim == Pglslang::EsdCube && sampler.isArrayed() && sampler.isShadow();

    // check for bias argument
    bool bias = false;
    if (! cracked.lod && ! cracked.grad && ! cracked.fetch && ! cubeCompare) {
        int nonBiasArgCount = 2;
        if (cracked.gather)
            ++nonBiasArgCount; // comp argument should be present when bias argument is present

        if (f16ShadowCompare)
            ++nonBiasArgCount;
        if (cracked.offset)
            ++nonBiasArgCount;
        else if (cracked.offsets)
            ++nonBiasArgCount;
        if (cracked.grad)
            nonBiasArgCount += 2;
        if (cracked.lodClamp)
            ++nonBiasArgCount;
        if (sparse)
            ++nonBiasArgCount;
        if (imageFootprint)
            //Following three extra arguments
            // int granularity, bool coarse, out gl_TextureFootprint2DNV footprint
            nonBiasArgCount += 3;
        if ((int)arguments.size() > nonBiasArgCount)
            bias = true;
    }

    // See if the sampler param should really be just the SPV image part
    if (cracked.fetch) {
        // a fetch needs to have the image extracted first
        if (builder.isSampledImage(params.sampler))
            params.sampler = builder.createUnaryOp(Pspv::OpImage, builder.getImageType(params.sampler), params.sampler);
    }

#ifndef GLSLANG_WEB
    if (cracked.gather) {
        const auto& sourceExtensions = glslangIntermediate->getRequestedExtensions();
        if (bias || cracked.lod ||
            sourceExtensions.find(Pglslang::E_GL_AMD_texture_gather_bias_lod) != sourceExtensions.end()) {
            builder.addExtension(Pspv::E_SPV_AMD_texture_gather_bias_lod);
            builder.addCapability(Pspv::CapabilityImageGatherBiasLodAMD);
        }
    }
#endif

    // set the rest of the arguments

    params.coords = arguments[1];
    int extraArgs = 0;
    bool noImplicitLod = false;

    // sort out where Dref is coming from
    if (cubeCompare || f16ShadowCompare) {
        params.Dref = arguments[2];
        ++extraArgs;
    } else if (sampler.shadow && cracked.gather) {
        params.Dref = arguments[2];
        ++extraArgs;
    } else if (sampler.shadow) {
        std::vector<Pspv::Id> indexes;
        int dRefComp;
        if (cracked.proj)
            dRefComp = 2;  // "The resulting 3rd component of P in the shadow forms is used as Dref"
        else
            dRefComp = builder.getNumComponents(params.coords) - 1;
        indexes.push_back(dRefComp);
        params.Dref = builder.createCompositeExtract(params.coords, builder.getScalarTypeId(builder.getTypeId(params.coords)), indexes);
    }

    // lod
    if (cracked.lod) {
        params.lod = arguments[2 + extraArgs];
        ++extraArgs;
    } else if (glslangIntermediate->getStage() != EShLangFragment &&
               !(glslangIntermediate->getStage() == EShLangCompute &&
                 glslangIntermediate->hasLayoutDerivativeModeNone())) {
        // we need to invent the default lod for an explicit lod instruction for a non-fragment stage
        noImplicitLod = true;
    }

    // multisample
    if (sampler.isMultiSample()) {
        params.sample = arguments[2 + extraArgs]; // For MS, "sample" should be specified
        ++extraArgs;
    }

    // gradient
    if (cracked.grad) {
        params.gradX = arguments[2 + extraArgs];
        params.gradY = arguments[3 + extraArgs];
        extraArgs += 2;
    }

    // offset and offsets
    if (cracked.offset) {
        params.offset = arguments[2 + extraArgs];
        ++extraArgs;
    } else if (cracked.offsets) {
        params.offsets = arguments[2 + extraArgs];
        ++extraArgs;
    }

#ifndef GLSLANG_WEB
    // lod clamp
    if (cracked.lodClamp) {
        params.lodClamp = arguments[2 + extraArgs];
        ++extraArgs;
    }
    // sparse
    if (sparse) {
        params.texelOut = arguments[2 + extraArgs];
        ++extraArgs;
    }
    // gather component
    if (cracked.gather && ! sampler.shadow) {
        // default component is 0, if missing, otherwise an argument
        if (2 + extraArgs < (int)arguments.size()) {
            params.component = arguments[2 + extraArgs];
            ++extraArgs;
        } else
            params.component = builder.makeIntConstant(0);
    }
    Pspv::Id  resultStruct = Pspv::NoResult;
    if (imageFootprint) {
        //Following three extra arguments
        // int granularity, bool coarse, out gl_TextureFootprint2DNV footprint
        params.granularity = arguments[2 + extraArgs];
        params.coarse = arguments[3 + extraArgs];
        resultStruct = arguments[4 + extraArgs];
        extraArgs += 3;
    }
#endif
    // bias
    if (bias) {
        params.bias = arguments[2 + extraArgs];
        ++extraArgs;
    }

#ifndef GLSLANG_WEB
    if (imageFootprint) {
        builder.addExtension(Pspv::E_SPV_NV_shader_image_footprint);
        builder.addCapability(Pspv::CapabilityImageFootprintNV);


        //resultStructType(OpenGL type) contains 5 elements:
        //struct gl_TextureFootprint2DNV {
        //    uvec2 anchor;
        //    uvec2 offset;
        //    uvec2 mask;
        //    uint  lod;
        //    uint  granularity;
        //};
        //or
        //struct gl_TextureFootprint3DNV {
        //    uvec3 anchor;
        //    uvec3 offset;
        //    uvec2 mask;
        //    uint  lod;
        //    uint  granularity;
        //};
        Pspv::Id resultStructType = builder.getContainedTypeId(builder.getTypeId(resultStruct));
        assert(builder.isStructType(resultStructType));

        //resType (SPIR-V type) contains 6 elements:
        //Member 0 must be a Boolean type scalar(LOD), 
        //Member 1 must be a vector of integer type, whose Signedness operand is 0(anchor),  
        //Member 2 must be a vector of integer type, whose Signedness operand is 0(offset), 
        //Member 3 must be a vector of integer type, whose Signedness operand is 0(mask), 
        //Member 4 must be a scalar of integer type, whose Signedness operand is 0(lod),
        //Member 5 must be a scalar of integer type, whose Signedness operand is 0(granularity).
        std::vector<Pspv::Id> members;
        members.push_back(resultType());
        for (int i = 0; i < 5; i++) {
            members.push_back(builder.getContainedTypeId(resultStructType, i));
        }
        Pspv::Id resType = builder.makeStructType(members, "ResType");

        //call ImageFootprintNV
        Pspv::Id res = builder.createTextureCall(precision, resType, sparse, cracked.fetch, cracked.proj,
                                                cracked.gather, noImplicitLod, params, signExtensionMask());
        
        //copy resType (SPIR-V type) to resultStructType(OpenGL type)
        for (int i = 0; i < 5; i++) {
            builder.clearAccessChain();
            builder.setAccessChainLValue(resultStruct);

            //Accessing to a struct we created, no coherent flag is set
            Pspv::Builder::AccessChain::CoherentFlags flags;
            flags.clear();

            builder.accessChainPush(builder.makeIntConstant(i), flags, 0);
            builder.accessChainStore(builder.createCompositeExtract(res, builder.getContainedTypeId(resType, i+1), i+1));
        }
        return builder.createCompositeExtract(res, resultType(), 0);
    }
#endif

    // projective component (might not to move)
    // GLSL: "The texture coordinates consumed from P, not including the last component of P,
    //       are divided by the last component of P."
    // SPIR-V:  "... (u [, v] [, w], q)... It may be a vector larger than needed, but all
    //          unused components will appear after all used components."
    if (cracked.proj) {
        int projSourceComp = builder.getNumComponents(params.coords) - 1;
        int projTargetComp;
        switch (sampler.dim) {
        case Pglslang::Esd1D:   projTargetComp = 1;              break;
        case Pglslang::Esd2D:   projTargetComp = 2;              break;
        case Pglslang::EsdRect: projTargetComp = 2;              break;
        default:               projTargetComp = projSourceComp; break;
        }
        // copy the projective coordinate if we have to
        if (projTargetComp != projSourceComp) {
            Pspv::Id projComp = builder.createCompositeExtract(params.coords,
                                                              builder.getScalarTypeId(builder.getTypeId(params.coords)),
                                                              projSourceComp);
            params.coords = builder.createCompositeInsert(projComp, params.coords,
                                                          builder.getTypeId(params.coords), projTargetComp);
        }
    }

#ifndef GLSLANG_WEB
    // nonprivate
    if (imageType.getQualifier().nonprivate) {
        params.nonprivate = true;
    }

    // volatile
    if (imageType.getQualifier().volatil) {
        params.volatil = true;
    }
#endif

    std::vector<Pspv::Id> result( 1, 
        builder.createTextureCall(precision, resultType(), sparse, cracked.fetch, cracked.proj, cracked.gather,
                                  noImplicitLod, params, signExtensionMask())
    );

    if (components != node->getType().getVectorSize())
        result[0] = builder.createConstructor(precision, result, convertGlslangToSpvType(node->getType()));

    return result[0];
}

Pspv::Id TGlslangToSpvTraverser::handleUserFunctionCall(const Pglslang::TIntermAggregate* node)
{
    // Grab the function's pointer from the previously created function
    Pspv::Function* function = functionMap[node->getName().c_str()];
    if (! function)
        return 0;

    const Pglslang::TIntermSequence& glslangArgs = node->getSequence();
    const Pglslang::TQualifierList& qualifiers = node->getQualifierList();

    //  See comments in makeFunctions() for details about the semantics for parameter passing.
    //
    // These imply we need a four step process:
    // 1. Evaluate the arguments
    // 2. Allocate and make copies of in, out, and inout arguments
    // 3. Make the call
    // 4. Copy back the results

    // 1. Evaluate the arguments and their types
    std::vector<Pspv::Builder::AccessChain> lValues;
    std::vector<Pspv::Id> rValues;
    std::vector<const Pglslang::TType*> argTypes;
    for (int a = 0; a < (int)glslangArgs.size(); ++a) {
        argTypes.push_back(&glslangArgs[a]->getAsTyped()->getType());
        // build l-value
        builder.clearAccessChain();
        glslangArgs[a]->traverse(this);
        // keep outputs and pass-by-originals as l-values, evaluate others as r-values
        if (originalParam(qualifiers[a], *argTypes[a], function->hasImplicitThis() && a == 0) ||
            writableParam(qualifiers[a])) {
            // save l-value
            lValues.push_back(builder.getAccessChain());
        } else {
            // process r-value
            rValues.push_back(accessChainLoad(*argTypes.back()));
        }
    }

    // 2. Allocate space for anything needing a copy, and if it's "in" or "inout"
    // copy the original into that space.
    //
    // Also, build up the list of actual arguments to pass in for the call
    int lValueCount = 0;
    int rValueCount = 0;
    std::vector<Pspv::Id> spvArgs;
    for (int a = 0; a < (int)glslangArgs.size(); ++a) {
        Pspv::Id arg;
        if (originalParam(qualifiers[a], *argTypes[a], function->hasImplicitThis() && a == 0)) {
            builder.setAccessChain(lValues[lValueCount]);
            arg = builder.accessChainGetLValue();
            ++lValueCount;
        } else if (writableParam(qualifiers[a])) {
            // need space to hold the copy
            arg = builder.createVariable(Pspv::StorageClassFunction, builder.getContainedTypeId(function->getParamType(a)), "param");
            if (qualifiers[a] == Pglslang::EvqIn || qualifiers[a] == Pglslang::EvqInOut) {
                // need to copy the input into output space
                builder.setAccessChain(lValues[lValueCount]);
                Pspv::Id copy = accessChainLoad(*argTypes[a]);
                builder.clearAccessChain();
                builder.setAccessChainLValue(arg);
                multiTypeStore(*argTypes[a], copy);
            }
            ++lValueCount;
        } else {
            // process r-value, which involves a copy for a type mismatch
            if (function->getParamType(a) != convertGlslangToSpvType(*argTypes[a])) {
                Pspv::Id argCopy = builder.createVariable(Pspv::StorageClassFunction, function->getParamType(a), "arg");
                builder.clearAccessChain();
                builder.setAccessChainLValue(argCopy);
                multiTypeStore(*argTypes[a], rValues[rValueCount]);
                arg = builder.createLoad(argCopy);
            } else
                arg = rValues[rValueCount];
            ++rValueCount;
        }
        spvArgs.push_back(arg);
    }

    // 3. Make the call.
    Pspv::Id result = builder.createFunctionCall(function, spvArgs);
    builder.setPrecision(result, TranslatePrecisionDecoration(node->getType()));

    // 4. Copy back out an "out" arguments.
    lValueCount = 0;
    for (int a = 0; a < (int)glslangArgs.size(); ++a) {
        if (originalParam(qualifiers[a], *argTypes[a], function->hasImplicitThis() && a == 0))
            ++lValueCount;
        else if (writableParam(qualifiers[a])) {
            if (qualifiers[a] == Pglslang::EvqOut || qualifiers[a] == Pglslang::EvqInOut) {
                Pspv::Id copy = builder.createLoad(spvArgs[a]);
                builder.setAccessChain(lValues[lValueCount]);
                multiTypeStore(*argTypes[a], copy);
            }
            ++lValueCount;
        }
    }

    return result;
}

// Translate AST operation to SPV operation, already having SPV-based operands/types.
Pspv::Id TGlslangToSpvTraverser::createBinaryOperation(Pglslang::TOperator op, OpDecorations& decorations,
                                                      Pspv::Id typeId, Pspv::Id left, Pspv::Id right,
                                                      Pglslang::TBasicType typeProxy, bool reduceComparison)
{
    bool isUnsigned = isTypeUnsignedInt(typeProxy);
    bool isFloat = isTypeFloat(typeProxy);
    bool isBool = typeProxy == Pglslang::EbtBool;

    Pspv::Op binOp = Pspv::OpNop;
    bool needMatchingVectors = true;  // for non-matrix ops, would a scalar need to smear to match a vector?
    bool comparison = false;

    switch (op) {
    case Pglslang::EOpAdd:
    case Pglslang::EOpAddAssign:
        if (isFloat)
            binOp = Pspv::OpFAdd;
        else
            binOp = Pspv::OpIAdd;
        break;
    case Pglslang::EOpSub:
    case Pglslang::EOpSubAssign:
        if (isFloat)
            binOp = Pspv::OpFSub;
        else
            binOp = Pspv::OpISub;
        break;
    case Pglslang::EOpMul:
    case Pglslang::EOpMulAssign:
        if (isFloat)
            binOp = Pspv::OpFMul;
        else
            binOp = Pspv::OpIMul;
        break;
    case Pglslang::EOpVectorTimesScalar:
    case Pglslang::EOpVectorTimesScalarAssign:
        if (isFloat && (builder.isVector(left) || builder.isVector(right))) {
            if (builder.isVector(right))
                std::swap(left, right);
            assert(builder.isScalar(right));
            needMatchingVectors = false;
            binOp = Pspv::OpVectorTimesScalar;
        } else if (isFloat)
            binOp = Pspv::OpFMul;
          else
            binOp = Pspv::OpIMul;
        break;
    case Pglslang::EOpVectorTimesMatrix:
    case Pglslang::EOpVectorTimesMatrixAssign:
        binOp = Pspv::OpVectorTimesMatrix;
        break;
    case Pglslang::EOpMatrixTimesVector:
        binOp = Pspv::OpMatrixTimesVector;
        break;
    case Pglslang::EOpMatrixTimesScalar:
    case Pglslang::EOpMatrixTimesScalarAssign:
        binOp = Pspv::OpMatrixTimesScalar;
        break;
    case Pglslang::EOpMatrixTimesMatrix:
    case Pglslang::EOpMatrixTimesMatrixAssign:
        binOp = Pspv::OpMatrixTimesMatrix;
        break;
    case Pglslang::EOpOuterProduct:
        binOp = Pspv::OpOuterProduct;
        needMatchingVectors = false;
        break;

    case Pglslang::EOpDiv:
    case Pglslang::EOpDivAssign:
        if (isFloat)
            binOp = Pspv::OpFDiv;
        else if (isUnsigned)
            binOp = Pspv::OpUDiv;
        else
            binOp = Pspv::OpSDiv;
        break;
    case Pglslang::EOpMod:
    case Pglslang::EOpModAssign:
        if (isFloat)
            binOp = Pspv::OpFMod;
        else if (isUnsigned)
            binOp = Pspv::OpUMod;
        else
            binOp = Pspv::OpSMod;
        break;
    case Pglslang::EOpRightShift:
    case Pglslang::EOpRightShiftAssign:
        if (isUnsigned)
            binOp = Pspv::OpShiftRightLogical;
        else
            binOp = Pspv::OpShiftRightArithmetic;
        break;
    case Pglslang::EOpLeftShift:
    case Pglslang::EOpLeftShiftAssign:
        binOp = Pspv::OpShiftLeftLogical;
        break;
    case Pglslang::EOpAnd:
    case Pglslang::EOpAndAssign:
        binOp = Pspv::OpBitwiseAnd;
        break;
    case Pglslang::EOpLogicalAnd:
        needMatchingVectors = false;
        binOp = Pspv::OpLogicalAnd;
        break;
    case Pglslang::EOpInclusiveOr:
    case Pglslang::EOpInclusiveOrAssign:
        binOp = Pspv::OpBitwiseOr;
        break;
    case Pglslang::EOpLogicalOr:
        needMatchingVectors = false;
        binOp = Pspv::OpLogicalOr;
        break;
    case Pglslang::EOpExclusiveOr:
    case Pglslang::EOpExclusiveOrAssign:
        binOp = Pspv::OpBitwiseXor;
        break;
    case Pglslang::EOpLogicalXor:
        needMatchingVectors = false;
        binOp = Pspv::OpLogicalNotEqual;
        break;

    case Pglslang::EOpAbsDifference:
        binOp = isUnsigned ? Pspv::OpAbsUSubINTEL : Pspv::OpAbsISubINTEL;
        break;

    case Pglslang::EOpAddSaturate:
        binOp = isUnsigned ? Pspv::OpUAddSatINTEL : Pspv::OpIAddSatINTEL;
        break;

    case Pglslang::EOpSubSaturate:
        binOp = isUnsigned ? Pspv::OpUSubSatINTEL : Pspv::OpISubSatINTEL;
        break;

    case Pglslang::EOpAverage:
        binOp = isUnsigned ? Pspv::OpUAverageINTEL : Pspv::OpIAverageINTEL;
        break;

    case Pglslang::EOpAverageRounded:
        binOp = isUnsigned ? Pspv::OpUAverageRoundedINTEL : Pspv::OpIAverageRoundedINTEL;
        break;

    case Pglslang::EOpMul32x16:
        binOp = isUnsigned ? Pspv::OpUMul32x16INTEL : Pspv::OpIMul32x16INTEL;
        break;

    case Pglslang::EOpLessThan:
    case Pglslang::EOpGreaterThan:
    case Pglslang::EOpLessThanEqual:
    case Pglslang::EOpGreaterThanEqual:
    case Pglslang::EOpEqual:
    case Pglslang::EOpNotEqual:
    case Pglslang::EOpVectorEqual:
    case Pglslang::EOpVectorNotEqual:
        comparison = true;
        break;
    default:
        break;
    }

    // handle mapped binary operations (should be non-comparison)
    if (binOp != Pspv::OpNop) {
        assert(comparison == false);
        if (builder.isMatrix(left) || builder.isMatrix(right) ||
            builder.isCooperativeMatrix(left) || builder.isCooperativeMatrix(right))
            return createBinaryMatrixOperation(binOp, decorations, typeId, left, right);

        // No matrix involved; make both operands be the same number of components, if needed
        if (needMatchingVectors)
            builder.promoteScalar(decorations.precision, left, right);

        Pspv::Id result = builder.createBinOp(binOp, typeId, left, right);
        decorations.addNoContraction(builder, result);
        decorations.addNonUniform(builder, result);
        return builder.setPrecision(result, decorations.precision);
    }

    if (! comparison)
        return 0;

    // Handle comparison instructions

    if (reduceComparison && (op == Pglslang::EOpEqual || op == Pglslang::EOpNotEqual)
                         && (builder.isVector(left) || builder.isMatrix(left) || builder.isAggregate(left))) {
        Pspv::Id result = builder.createCompositeCompare(decorations.precision, left, right, op == Pglslang::EOpEqual);
        decorations.addNonUniform(builder, result);
        return result;
    }

    switch (op) {
    case Pglslang::EOpLessThan:
        if (isFloat)
            binOp = Pspv::OpFOrdLessThan;
        else if (isUnsigned)
            binOp = Pspv::OpULessThan;
        else
            binOp = Pspv::OpSLessThan;
        break;
    case Pglslang::EOpGreaterThan:
        if (isFloat)
            binOp = Pspv::OpFOrdGreaterThan;
        else if (isUnsigned)
            binOp = Pspv::OpUGreaterThan;
        else
            binOp = Pspv::OpSGreaterThan;
        break;
    case Pglslang::EOpLessThanEqual:
        if (isFloat)
            binOp = Pspv::OpFOrdLessThanEqual;
        else if (isUnsigned)
            binOp = Pspv::OpULessThanEqual;
        else
            binOp = Pspv::OpSLessThanEqual;
        break;
    case Pglslang::EOpGreaterThanEqual:
        if (isFloat)
            binOp = Pspv::OpFOrdGreaterThanEqual;
        else if (isUnsigned)
            binOp = Pspv::OpUGreaterThanEqual;
        else
            binOp = Pspv::OpSGreaterThanEqual;
        break;
    case Pglslang::EOpEqual:
    case Pglslang::EOpVectorEqual:
        if (isFloat)
            binOp = Pspv::OpFOrdEqual;
        else if (isBool)
            binOp = Pspv::OpLogicalEqual;
        else
            binOp = Pspv::OpIEqual;
        break;
    case Pglslang::EOpNotEqual:
    case Pglslang::EOpVectorNotEqual:
        if (isFloat)
            binOp = Pspv::OpFOrdNotEqual;
        else if (isBool)
            binOp = Pspv::OpLogicalNotEqual;
        else
            binOp = Pspv::OpINotEqual;
        break;
    default:
        break;
    }

    if (binOp != Pspv::OpNop) {
        Pspv::Id result = builder.createBinOp(binOp, typeId, left, right);
        decorations.addNoContraction(builder, result);
        decorations.addNonUniform(builder, result);
        return builder.setPrecision(result, decorations.precision);
    }

    return 0;
}

//
// Translate AST matrix operation to SPV operation, already having SPV-based operands/types.
// These can be any of:
//
//   matrix * scalar
//   scalar * matrix
//   matrix * matrix     linear algebraic
//   matrix * vector
//   vector * matrix
//   matrix * matrix     componentwise
//   matrix op matrix    op in {+, -, /}
//   matrix op scalar    op in {+, -, /}
//   scalar op matrix    op in {+, -, /}
//
Pspv::Id TGlslangToSpvTraverser::createBinaryMatrixOperation(Pspv::Op op, OpDecorations& decorations, Pspv::Id typeId,
                                                            Pspv::Id left, Pspv::Id right)
{
    bool firstClass = true;

    // First, handle first-class matrix operations (* and matrix/scalar)
    switch (op) {
    case Pspv::OpFDiv:
        if (builder.isMatrix(left) && builder.isScalar(right)) {
            // turn matrix / scalar into a multiply...
            Pspv::Id resultType = builder.getTypeId(right);
            right = builder.createBinOp(Pspv::OpFDiv, resultType, builder.makeFpConstant(resultType, 1.0), right);
            op = Pspv::OpMatrixTimesScalar;
        } else
            firstClass = false;
        break;
    case Pspv::OpMatrixTimesScalar:
        if (builder.isMatrix(right) || builder.isCooperativeMatrix(right))
            std::swap(left, right);
        assert(builder.isScalar(right));
        break;
    case Pspv::OpVectorTimesMatrix:
        assert(builder.isVector(left));
        assert(builder.isMatrix(right));
        break;
    case Pspv::OpMatrixTimesVector:
        assert(builder.isMatrix(left));
        assert(builder.isVector(right));
        break;
    case Pspv::OpMatrixTimesMatrix:
        assert(builder.isMatrix(left));
        assert(builder.isMatrix(right));
        break;
    default:
        firstClass = false;
        break;
    }

    if (builder.isCooperativeMatrix(left) || builder.isCooperativeMatrix(right))
        firstClass = true;

    if (firstClass) {
        Pspv::Id result = builder.createBinOp(op, typeId, left, right);
        decorations.addNoContraction(builder, result);
        decorations.addNonUniform(builder, result);
        return builder.setPrecision(result, decorations.precision);
    }

    // Handle component-wise +, -, *, %, and / for all combinations of type.
    // The result type of all of them is the same type as the (a) matrix operand.
    // The algorithm is to:
    //   - break the matrix(es) into vectors
    //   - smear any scalar to a vector
    //   - do vector operations
    //   - make a matrix out the vector results
    switch (op) {
    case Pspv::OpFAdd:
    case Pspv::OpFSub:
    case Pspv::OpFDiv:
    case Pspv::OpFMod:
    case Pspv::OpFMul:
    {
        // one time set up...
        bool  leftMat = builder.isMatrix(left);
        bool rightMat = builder.isMatrix(right);
        unsigned int numCols = leftMat ? builder.getNumColumns(left) : builder.getNumColumns(right);
        int numRows = leftMat ? builder.getNumRows(left) : builder.getNumRows(right);
        Pspv::Id scalarType = builder.getScalarTypeId(typeId);
        Pspv::Id vecType = builder.makeVectorType(scalarType, numRows);
        std::vector<Pspv::Id> results;
        Pspv::Id smearVec = Pspv::NoResult;
        if (builder.isScalar(left))
            smearVec = builder.smearScalar(decorations.precision, left, vecType);
        else if (builder.isScalar(right))
            smearVec = builder.smearScalar(decorations.precision, right, vecType);

        // do each vector op
        for (unsigned int c = 0; c < numCols; ++c) {
            std::vector<unsigned int> indexes;
            indexes.push_back(c);
            Pspv::Id  leftVec =  leftMat ? builder.createCompositeExtract( left, vecType, indexes) : smearVec;
            Pspv::Id rightVec = rightMat ? builder.createCompositeExtract(right, vecType, indexes) : smearVec;
            Pspv::Id result = builder.createBinOp(op, vecType, leftVec, rightVec);
            decorations.addNoContraction(builder, result);
            decorations.addNonUniform(builder, result);
            results.push_back(builder.setPrecision(result, decorations.precision));
        }

        // put the pieces together
        Pspv::Id result = builder.setPrecision(builder.createCompositeConstruct(typeId, results), decorations.precision);
        decorations.addNonUniform(builder, result);
        return result;
    }
    default:
        assert(0);
        return Pspv::NoResult;
    }
}

Pspv::Id TGlslangToSpvTraverser::createUnaryOperation(Pglslang::TOperator op, OpDecorations& decorations, Pspv::Id typeId,
                                                     Pspv::Id operand, Pglslang::TBasicType typeProxy, const Pspv::Builder::AccessChain::CoherentFlags &lvalueCoherentFlags)
{
    Pspv::Op unaryOp = Pspv::OpNop;
    int extBuiltins = -1;
    int libCall = -1;
    bool isUnsigned = isTypeUnsignedInt(typeProxy);
    bool isFloat = isTypeFloat(typeProxy);

    switch (op) {
    case Pglslang::EOpNegative:
        if (isFloat) {
            unaryOp = Pspv::OpFNegate;
            if (builder.isMatrixType(typeId))
                return createUnaryMatrixOperation(unaryOp, decorations, typeId, operand, typeProxy);
        } else
            unaryOp = Pspv::OpSNegate;
        break;

    case Pglslang::EOpLogicalNot:
    case Pglslang::EOpVectorLogicalNot:
        unaryOp = Pspv::OpLogicalNot;
        break;
    case Pglslang::EOpBitwiseNot:
        unaryOp = Pspv::OpNot;
        break;

    case Pglslang::EOpDeterminant:
        libCall = Pspv::GLSLstd450Determinant;
        break;
    case Pglslang::EOpMatrixInverse:
        libCall = Pspv::GLSLstd450MatrixInverse;
        break;
    case Pglslang::EOpTranspose:
        unaryOp = Pspv::OpTranspose;
        break;

    case Pglslang::EOpRadians:
        libCall = Pspv::GLSLstd450Radians;
        break;
    case Pglslang::EOpDegrees:
        libCall = Pspv::GLSLstd450Degrees;
        break;
    case Pglslang::EOpSin:
        libCall = Pspv::GLSLstd450Sin;
        break;
    case Pglslang::EOpCos:
        libCall = Pspv::GLSLstd450Cos;
        break;
    case Pglslang::EOpTan:
        libCall = Pspv::GLSLstd450Tan;
        break;
    case Pglslang::EOpAcos:
        libCall = Pspv::GLSLstd450Acos;
        break;
    case Pglslang::EOpAsin:
        libCall = Pspv::GLSLstd450Asin;
        break;
    case Pglslang::EOpAtan:
        libCall = Pspv::GLSLstd450Atan;
        break;

    case Pglslang::EOpAcosh:
        libCall = Pspv::GLSLstd450Acosh;
        break;
    case Pglslang::EOpAsinh:
        libCall = Pspv::GLSLstd450Asinh;
        break;
    case Pglslang::EOpAtanh:
        libCall = Pspv::GLSLstd450Atanh;
        break;
    case Pglslang::EOpTanh:
        libCall = Pspv::GLSLstd450Tanh;
        break;
    case Pglslang::EOpCosh:
        libCall = Pspv::GLSLstd450Cosh;
        break;
    case Pglslang::EOpSinh:
        libCall = Pspv::GLSLstd450Sinh;
        break;

    case Pglslang::EOpLength:
        libCall = Pspv::GLSLstd450Length;
        break;
    case Pglslang::EOpNormalize:
        libCall = Pspv::GLSLstd450Normalize;
        break;

    case Pglslang::EOpExp:
        libCall = Pspv::GLSLstd450Exp;
        break;
    case Pglslang::EOpLog:
        libCall = Pspv::GLSLstd450Log;
        break;
    case Pglslang::EOpExp2:
        libCall = Pspv::GLSLstd450Exp2;
        break;
    case Pglslang::EOpLog2:
        libCall = Pspv::GLSLstd450Log2;
        break;
    case Pglslang::EOpSqrt:
        libCall = Pspv::GLSLstd450Sqrt;
        break;
    case Pglslang::EOpInverseSqrt:
        libCall = Pspv::GLSLstd450InverseSqrt;
        break;

    case Pglslang::EOpFloor:
        libCall = Pspv::GLSLstd450Floor;
        break;
    case Pglslang::EOpTrunc:
        libCall = Pspv::GLSLstd450Trunc;
        break;
    case Pglslang::EOpRound:
        libCall = Pspv::GLSLstd450Round;
        break;
    case Pglslang::EOpRoundEven:
        libCall = Pspv::GLSLstd450RoundEven;
        break;
    case Pglslang::EOpCeil:
        libCall = Pspv::GLSLstd450Ceil;
        break;
    case Pglslang::EOpFract:
        libCall = Pspv::GLSLstd450Fract;
        break;

    case Pglslang::EOpIsNan:
        unaryOp = Pspv::OpIsNan;
        break;
    case Pglslang::EOpIsInf:
        unaryOp = Pspv::OpIsInf;
        break;
    case Pglslang::EOpIsFinite:
        unaryOp = Pspv::OpIsFinite;
        break;

    case Pglslang::EOpFloatBitsToInt:
    case Pglslang::EOpFloatBitsToUint:
    case Pglslang::EOpIntBitsToFloat:
    case Pglslang::EOpUintBitsToFloat:
    case Pglslang::EOpDoubleBitsToInt64:
    case Pglslang::EOpDoubleBitsToUint64:
    case Pglslang::EOpInt64BitsToDouble:
    case Pglslang::EOpUint64BitsToDouble:
    case Pglslang::EOpFloat16BitsToInt16:
    case Pglslang::EOpFloat16BitsToUint16:
    case Pglslang::EOpInt16BitsToFloat16:
    case Pglslang::EOpUint16BitsToFloat16:
        unaryOp = Pspv::OpBitcast;
        break;

    case Pglslang::EOpPackSnorm2x16:
        libCall = Pspv::GLSLstd450PackSnorm2x16;
        break;
    case Pglslang::EOpUnpackSnorm2x16:
        libCall = Pspv::GLSLstd450UnpackSnorm2x16;
        break;
    case Pglslang::EOpPackUnorm2x16:
        libCall = Pspv::GLSLstd450PackUnorm2x16;
        break;
    case Pglslang::EOpUnpackUnorm2x16:
        libCall = Pspv::GLSLstd450UnpackUnorm2x16;
        break;
    case Pglslang::EOpPackHalf2x16:
        libCall = Pspv::GLSLstd450PackHalf2x16;
        break;
    case Pglslang::EOpUnpackHalf2x16:
        libCall = Pspv::GLSLstd450UnpackHalf2x16;
        break;
#ifndef GLSLANG_WEB
    case Pglslang::EOpPackSnorm4x8:
        libCall = Pspv::GLSLstd450PackSnorm4x8;
        break;
    case Pglslang::EOpUnpackSnorm4x8:
        libCall = Pspv::GLSLstd450UnpackSnorm4x8;
        break;
    case Pglslang::EOpPackUnorm4x8:
        libCall = Pspv::GLSLstd450PackUnorm4x8;
        break;
    case Pglslang::EOpUnpackUnorm4x8:
        libCall = Pspv::GLSLstd450UnpackUnorm4x8;
        break;
    case Pglslang::EOpPackDouble2x32:
        libCall = Pspv::GLSLstd450PackDouble2x32;
        break;
    case Pglslang::EOpUnpackDouble2x32:
        libCall = Pspv::GLSLstd450UnpackDouble2x32;
        break;
#endif

    case Pglslang::EOpPackInt2x32:
    case Pglslang::EOpUnpackInt2x32:
    case Pglslang::EOpPackUint2x32:
    case Pglslang::EOpUnpackUint2x32:
    case Pglslang::EOpPack16:
    case Pglslang::EOpPack32:
    case Pglslang::EOpPack64:
    case Pglslang::EOpUnpack32:
    case Pglslang::EOpUnpack16:
    case Pglslang::EOpUnpack8:
    case Pglslang::EOpPackInt2x16:
    case Pglslang::EOpUnpackInt2x16:
    case Pglslang::EOpPackUint2x16:
    case Pglslang::EOpUnpackUint2x16:
    case Pglslang::EOpPackInt4x16:
    case Pglslang::EOpUnpackInt4x16:
    case Pglslang::EOpPackUint4x16:
    case Pglslang::EOpUnpackUint4x16:
    case Pglslang::EOpPackFloat2x16:
    case Pglslang::EOpUnpackFloat2x16:
        unaryOp = Pspv::OpBitcast;
        break;

    case Pglslang::EOpDPdx:
        unaryOp = Pspv::OpDPdx;
        break;
    case Pglslang::EOpDPdy:
        unaryOp = Pspv::OpDPdy;
        break;
    case Pglslang::EOpFwidth:
        unaryOp = Pspv::OpFwidth;
        break;

    case Pglslang::EOpAny:
        unaryOp = Pspv::OpAny;
        break;
    case Pglslang::EOpAll:
        unaryOp = Pspv::OpAll;
        break;

    case Pglslang::EOpAbs:
        if (isFloat)
            libCall = Pspv::GLSLstd450FAbs;
        else
            libCall = Pspv::GLSLstd450SAbs;
        break;
    case Pglslang::EOpSign:
        if (isFloat)
            libCall = Pspv::GLSLstd450FSign;
        else
            libCall = Pspv::GLSLstd450SSign;
        break;

#ifndef GLSLANG_WEB
    case Pglslang::EOpDPdxFine:
        unaryOp = Pspv::OpDPdxFine;
        break;
    case Pglslang::EOpDPdyFine:
        unaryOp = Pspv::OpDPdyFine;
        break;
    case Pglslang::EOpFwidthFine:
        unaryOp = Pspv::OpFwidthFine;
        break;
    case Pglslang::EOpDPdxCoarse:
        unaryOp = Pspv::OpDPdxCoarse;
        break;
    case Pglslang::EOpDPdyCoarse:
        unaryOp = Pspv::OpDPdyCoarse;
        break;
    case Pglslang::EOpFwidthCoarse:
        unaryOp = Pspv::OpFwidthCoarse;
        break;
    case Pglslang::EOpInterpolateAtCentroid:
        if (typeProxy == Pglslang::EbtFloat16)
            builder.addExtension(Pspv::E_SPV_AMD_gpu_shader_half_float);
        libCall = Pspv::GLSLstd450InterpolateAtCentroid;
        break;
    case Pglslang::EOpAtomicCounterIncrement:
    case Pglslang::EOpAtomicCounterDecrement:
    case Pglslang::EOpAtomicCounter:
    {
        // Handle all of the atomics in one place, in createAtomicOperation()
        std::vector<Pspv::Id> operands;
        operands.push_back(operand);
        return createAtomicOperation(op, decorations.precision, typeId, operands, typeProxy, lvalueCoherentFlags);
    }

    case Pglslang::EOpBitFieldReverse:
        unaryOp = Pspv::OpBitReverse;
        break;
    case Pglslang::EOpBitCount:
        unaryOp = Pspv::OpBitCount;
        break;
    case Pglslang::EOpFindLSB:
        libCall = Pspv::GLSLstd450FindILsb;
        break;
    case Pglslang::EOpFindMSB:
        if (isUnsigned)
            libCall = Pspv::GLSLstd450FindUMsb;
        else
            libCall = Pspv::GLSLstd450FindSMsb;
        break;

    case Pglslang::EOpCountLeadingZeros:
        builder.addCapability(Pspv::CapabilityIntegerFunctions2INTEL);
        builder.addExtension("SPV_INTEL_shader_integer_functions2");
        unaryOp = Pspv::OpUCountLeadingZerosINTEL;
        break;

    case Pglslang::EOpCountTrailingZeros:
        builder.addCapability(Pspv::CapabilityIntegerFunctions2INTEL);
        builder.addExtension("SPV_INTEL_shader_integer_functions2");
        unaryOp = Pspv::OpUCountTrailingZerosINTEL;
        break;

    case Pglslang::EOpBallot:
    case Pglslang::EOpReadFirstInvocation:
    case Pglslang::EOpAnyInvocation:
    case Pglslang::EOpAllInvocations:
    case Pglslang::EOpAllInvocationsEqual:
    case Pglslang::EOpMinInvocations:
    case Pglslang::EOpMaxInvocations:
    case Pglslang::EOpAddInvocations:
    case Pglslang::EOpMinInvocationsNonUniform:
    case Pglslang::EOpMaxInvocationsNonUniform:
    case Pglslang::EOpAddInvocationsNonUniform:
    case Pglslang::EOpMinInvocationsInclusiveScan:
    case Pglslang::EOpMaxInvocationsInclusiveScan:
    case Pglslang::EOpAddInvocationsInclusiveScan:
    case Pglslang::EOpMinInvocationsInclusiveScanNonUniform:
    case Pglslang::EOpMaxInvocationsInclusiveScanNonUniform:
    case Pglslang::EOpAddInvocationsInclusiveScanNonUniform:
    case Pglslang::EOpMinInvocationsExclusiveScan:
    case Pglslang::EOpMaxInvocationsExclusiveScan:
    case Pglslang::EOpAddInvocationsExclusiveScan:
    case Pglslang::EOpMinInvocationsExclusiveScanNonUniform:
    case Pglslang::EOpMaxInvocationsExclusiveScanNonUniform:
    case Pglslang::EOpAddInvocationsExclusiveScanNonUniform:
    {
        std::vector<Pspv::Id> operands;
        operands.push_back(operand);
        return createInvocationsOperation(op, typeId, operands, typeProxy);
    }
    case Pglslang::EOpSubgroupAll:
    case Pglslang::EOpSubgroupAny:
    case Pglslang::EOpSubgroupAllEqual:
    case Pglslang::EOpSubgroupBroadcastFirst:
    case Pglslang::EOpSubgroupBallot:
    case Pglslang::EOpSubgroupInverseBallot:
    case Pglslang::EOpSubgroupBallotBitCount:
    case Pglslang::EOpSubgroupBallotInclusiveBitCount:
    case Pglslang::EOpSubgroupBallotExclusiveBitCount:
    case Pglslang::EOpSubgroupBallotFindLSB:
    case Pglslang::EOpSubgroupBallotFindMSB:
    case Pglslang::EOpSubgroupAdd:
    case Pglslang::EOpSubgroupMul:
    case Pglslang::EOpSubgroupMin:
    case Pglslang::EOpSubgroupMax:
    case Pglslang::EOpSubgroupAnd:
    case Pglslang::EOpSubgroupOr:
    case Pglslang::EOpSubgroupXor:
    case Pglslang::EOpSubgroupInclusiveAdd:
    case Pglslang::EOpSubgroupInclusiveMul:
    case Pglslang::EOpSubgroupInclusiveMin:
    case Pglslang::EOpSubgroupInclusiveMax:
    case Pglslang::EOpSubgroupInclusiveAnd:
    case Pglslang::EOpSubgroupInclusiveOr:
    case Pglslang::EOpSubgroupInclusiveXor:
    case Pglslang::EOpSubgroupExclusiveAdd:
    case Pglslang::EOpSubgroupExclusiveMul:
    case Pglslang::EOpSubgroupExclusiveMin:
    case Pglslang::EOpSubgroupExclusiveMax:
    case Pglslang::EOpSubgroupExclusiveAnd:
    case Pglslang::EOpSubgroupExclusiveOr:
    case Pglslang::EOpSubgroupExclusiveXor:
    case Pglslang::EOpSubgroupQuadSwapHorizontal:
    case Pglslang::EOpSubgroupQuadSwapVertical:
    case Pglslang::EOpSubgroupQuadSwapDiagonal: {
        std::vector<Pspv::Id> operands;
        operands.push_back(operand);
        return createSubgroupOperation(op, typeId, operands, typeProxy);
    }
    case Pglslang::EOpMbcnt:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_shader_ballot);
        libCall = Pspv::MbcntAMD;
        break;

    case Pglslang::EOpCubeFaceIndex:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_gcn_shader);
        libCall = Pspv::CubeFaceIndexAMD;
        break;

    case Pglslang::EOpCubeFaceCoord:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_gcn_shader);
        libCall = Pspv::CubeFaceCoordAMD;
        break;
    case Pglslang::EOpSubgroupPartition:
        unaryOp = Pspv::OpGroupNonUniformPartitionNV;
        break;
    case Pglslang::EOpConstructReference:
        unaryOp = Pspv::OpBitcast;
        break;
#endif

    case Pglslang::EOpCopyObject:
        unaryOp = Pspv::OpCopyObject;
        break;

    default:
        return 0;
    }

    Pspv::Id id;
    if (libCall >= 0) {
        std::vector<Pspv::Id> args;
        args.push_back(operand);
        id = builder.createBuiltinCall(typeId, extBuiltins >= 0 ? extBuiltins : stdBuiltins, libCall, args);
    } else {
        id = builder.createUnaryOp(unaryOp, typeId, operand);
    }

    decorations.addNoContraction(builder, id);
    decorations.addNonUniform(builder, id);
    return builder.setPrecision(id, decorations.precision);
}

// Create a unary operation on a matrix
Pspv::Id TGlslangToSpvTraverser::createUnaryMatrixOperation(Pspv::Op op, OpDecorations& decorations, Pspv::Id typeId,
                                                           Pspv::Id operand, Pglslang::TBasicType /* typeProxy */)
{
    // Handle unary operations vector by vector.
    // The result type is the same type as the original type.
    // The algorithm is to:
    //   - break the matrix into vectors
    //   - apply the operation to each vector
    //   - make a matrix out the vector results

    // get the types sorted out
    int numCols = builder.getNumColumns(operand);
    int numRows = builder.getNumRows(operand);
    Pspv::Id srcVecType  = builder.makeVectorType(builder.getScalarTypeId(builder.getTypeId(operand)), numRows);
    Pspv::Id destVecType = builder.makeVectorType(builder.getScalarTypeId(typeId), numRows);
    std::vector<Pspv::Id> results;

    // do each vector op
    for (int c = 0; c < numCols; ++c) {
        std::vector<unsigned int> indexes;
        indexes.push_back(c);
        Pspv::Id srcVec  = builder.createCompositeExtract(operand, srcVecType, indexes);
        Pspv::Id destVec = builder.createUnaryOp(op, destVecType, srcVec);
        decorations.addNoContraction(builder, destVec);
        decorations.addNonUniform(builder, destVec);
        results.push_back(builder.setPrecision(destVec, decorations.precision));
    }

    // put the pieces together
    Pspv::Id result = builder.setPrecision(builder.createCompositeConstruct(typeId, results), decorations.precision);
    decorations.addNonUniform(builder, result);
    return result;
}

// For converting integers where both the bitwidth and the signedness could
// change, but only do the width change here. The caller is still responsible
// for the signedness conversion.
Pspv::Id TGlslangToSpvTraverser::createIntWidthConversion(Pglslang::TOperator op, Pspv::Id operand, int vectorSize)
{
    // Get the result type width, based on the type to convert to.
    int width = 32;
    switch(op) {
    case Pglslang::EOpConvInt16ToUint8:
    case Pglslang::EOpConvIntToUint8:
    case Pglslang::EOpConvInt64ToUint8:
    case Pglslang::EOpConvUint16ToInt8:
    case Pglslang::EOpConvUintToInt8:
    case Pglslang::EOpConvUint64ToInt8:
        width = 8;
        break;
    case Pglslang::EOpConvInt8ToUint16:
    case Pglslang::EOpConvIntToUint16:
    case Pglslang::EOpConvInt64ToUint16:
    case Pglslang::EOpConvUint8ToInt16:
    case Pglslang::EOpConvUintToInt16:
    case Pglslang::EOpConvUint64ToInt16:
        width = 16;
        break;
    case Pglslang::EOpConvInt8ToUint:
    case Pglslang::EOpConvInt16ToUint:
    case Pglslang::EOpConvInt64ToUint:
    case Pglslang::EOpConvUint8ToInt:
    case Pglslang::EOpConvUint16ToInt:
    case Pglslang::EOpConvUint64ToInt:
        width = 32;
        break;
    case Pglslang::EOpConvInt8ToUint64:
    case Pglslang::EOpConvInt16ToUint64:
    case Pglslang::EOpConvIntToUint64:
    case Pglslang::EOpConvUint8ToInt64:
    case Pglslang::EOpConvUint16ToInt64:
    case Pglslang::EOpConvUintToInt64:
        width = 64;
        break;

    default:
        assert(false && "Default missing");
        break;
    }

    // Get the conversion operation and result type,
    // based on the target width, but the source type.
    Pspv::Id type = Pspv::NoType;
    Pspv::Op convOp = Pspv::OpNop;
    switch(op) {
    case Pglslang::EOpConvInt8ToUint16:
    case Pglslang::EOpConvInt8ToUint:
    case Pglslang::EOpConvInt8ToUint64:
    case Pglslang::EOpConvInt16ToUint8:
    case Pglslang::EOpConvInt16ToUint:
    case Pglslang::EOpConvInt16ToUint64:
    case Pglslang::EOpConvIntToUint8:
    case Pglslang::EOpConvIntToUint16:
    case Pglslang::EOpConvIntToUint64:
    case Pglslang::EOpConvInt64ToUint8:
    case Pglslang::EOpConvInt64ToUint16:
    case Pglslang::EOpConvInt64ToUint:
        convOp = Pspv::OpSConvert;
        type = builder.makeIntType(width);
        break;
    default:
        convOp = Pspv::OpUConvert;
        type = builder.makeUintType(width);
        break;
    }

    if (vectorSize > 0)
        type = builder.makeVectorType(type, vectorSize);

    return builder.createUnaryOp(convOp, type, operand);
}

Pspv::Id TGlslangToSpvTraverser::createConversion(Pglslang::TOperator op, OpDecorations& decorations, Pspv::Id destType,
                                                 Pspv::Id operand, Pglslang::TBasicType typeProxy)
{
    Pspv::Op convOp = Pspv::OpNop;
    Pspv::Id zero = 0;
    Pspv::Id one = 0;

    int vectorSize = builder.isVectorType(destType) ? builder.getNumTypeComponents(destType) : 0;

    switch (op) {
    case Pglslang::EOpConvIntToBool:
    case Pglslang::EOpConvUintToBool:
        zero = builder.makeUintConstant(0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(Pspv::OpINotEqual, destType, operand, zero);
    case Pglslang::EOpConvFloatToBool:
        zero = builder.makeFloatConstant(0.0F);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(Pspv::OpFOrdNotEqual, destType, operand, zero);
    case Pglslang::EOpConvBoolToFloat:
        convOp = Pspv::OpSelect;
        zero = builder.makeFloatConstant(0.0F);
        one  = builder.makeFloatConstant(1.0F);
        break;

    case Pglslang::EOpConvBoolToInt:
    case Pglslang::EOpConvBoolToInt64:
#ifndef GLSLANG_WEB
        if (op == Pglslang::EOpConvBoolToInt64) {
            zero = builder.makeInt64Constant(0);
            one = builder.makeInt64Constant(1);
        } else
#endif
        {
            zero = builder.makeIntConstant(0);
            one = builder.makeIntConstant(1);
        }

        convOp = Pspv::OpSelect;
        break;

    case Pglslang::EOpConvBoolToUint:
    case Pglslang::EOpConvBoolToUint64:
#ifndef GLSLANG_WEB
        if (op == Pglslang::EOpConvBoolToUint64) {
            zero = builder.makeUint64Constant(0);
            one = builder.makeUint64Constant(1);
        } else
#endif
        {
            zero = builder.makeUintConstant(0);
            one = builder.makeUintConstant(1);
        }

        convOp = Pspv::OpSelect;
        break;

    case Pglslang::EOpConvInt8ToFloat16:
    case Pglslang::EOpConvInt8ToFloat:
    case Pglslang::EOpConvInt8ToDouble:
    case Pglslang::EOpConvInt16ToFloat16:
    case Pglslang::EOpConvInt16ToFloat:
    case Pglslang::EOpConvInt16ToDouble:
    case Pglslang::EOpConvIntToFloat16:
    case Pglslang::EOpConvIntToFloat:
    case Pglslang::EOpConvIntToDouble:
    case Pglslang::EOpConvInt64ToFloat:
    case Pglslang::EOpConvInt64ToDouble:
    case Pglslang::EOpConvInt64ToFloat16:
        convOp = Pspv::OpConvertSToF;
        break;

    case Pglslang::EOpConvUint8ToFloat16:
    case Pglslang::EOpConvUint8ToFloat:
    case Pglslang::EOpConvUint8ToDouble:
    case Pglslang::EOpConvUint16ToFloat16:
    case Pglslang::EOpConvUint16ToFloat:
    case Pglslang::EOpConvUint16ToDouble:
    case Pglslang::EOpConvUintToFloat16:
    case Pglslang::EOpConvUintToFloat:
    case Pglslang::EOpConvUintToDouble:
    case Pglslang::EOpConvUint64ToFloat:
    case Pglslang::EOpConvUint64ToDouble:
    case Pglslang::EOpConvUint64ToFloat16:
        convOp = Pspv::OpConvertUToF;
        break;

    case Pglslang::EOpConvFloat16ToInt8:
    case Pglslang::EOpConvFloatToInt8:
    case Pglslang::EOpConvDoubleToInt8:
    case Pglslang::EOpConvFloat16ToInt16:
    case Pglslang::EOpConvFloatToInt16:
    case Pglslang::EOpConvDoubleToInt16:
    case Pglslang::EOpConvFloat16ToInt:
    case Pglslang::EOpConvFloatToInt:
    case Pglslang::EOpConvDoubleToInt:
    case Pglslang::EOpConvFloat16ToInt64:
    case Pglslang::EOpConvFloatToInt64:
    case Pglslang::EOpConvDoubleToInt64:
        convOp = Pspv::OpConvertFToS;
        break;

    case Pglslang::EOpConvUint8ToInt8:
    case Pglslang::EOpConvInt8ToUint8:
    case Pglslang::EOpConvUint16ToInt16:
    case Pglslang::EOpConvInt16ToUint16:
    case Pglslang::EOpConvUintToInt:
    case Pglslang::EOpConvIntToUint:
    case Pglslang::EOpConvUint64ToInt64:
    case Pglslang::EOpConvInt64ToUint64:
        if (builder.isInSpecConstCodeGenMode()) {
            // Build zero scalar or vector for OpIAdd.
#ifndef GLSLANG_WEB
            if(op == Pglslang::EOpConvUint8ToInt8 || op == Pglslang::EOpConvInt8ToUint8) {
                zero = builder.makeUint8Constant(0);
            } else if (op == Pglslang::EOpConvUint16ToInt16 || op == Pglslang::EOpConvInt16ToUint16) {
                zero = builder.makeUint16Constant(0);
            } else if (op == Pglslang::EOpConvUint64ToInt64 || op == Pglslang::EOpConvInt64ToUint64) {
                zero = builder.makeUint64Constant(0);
            } else
#endif
            {
                zero = builder.makeUintConstant(0);
            }
            zero = makeSmearedConstant(zero, vectorSize);
            // Use OpIAdd, instead of OpBitcast to do the conversion when
            // generating for OpSpecConstantOp instruction.
            return builder.createBinOp(Pspv::OpIAdd, destType, operand, zero);
        }
        // For normal run-time conversion instruction, use OpBitcast.
        convOp = Pspv::OpBitcast;
        break;

    case Pglslang::EOpConvFloat16ToUint8:
    case Pglslang::EOpConvFloatToUint8:
    case Pglslang::EOpConvDoubleToUint8:
    case Pglslang::EOpConvFloat16ToUint16:
    case Pglslang::EOpConvFloatToUint16:
    case Pglslang::EOpConvDoubleToUint16:
    case Pglslang::EOpConvFloat16ToUint:
    case Pglslang::EOpConvFloatToUint:
    case Pglslang::EOpConvDoubleToUint:
    case Pglslang::EOpConvFloatToUint64:
    case Pglslang::EOpConvDoubleToUint64:
    case Pglslang::EOpConvFloat16ToUint64:
        convOp = Pspv::OpConvertFToU;
        break;

#ifndef GLSLANG_WEB
    case Pglslang::EOpConvInt8ToBool:
    case Pglslang::EOpConvUint8ToBool:
        zero = builder.makeUint8Constant(0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(Pspv::OpINotEqual, destType, operand, zero);
    case Pglslang::EOpConvInt16ToBool:
    case Pglslang::EOpConvUint16ToBool:
        zero = builder.makeUint16Constant(0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(Pspv::OpINotEqual, destType, operand, zero);
    case Pglslang::EOpConvInt64ToBool:
    case Pglslang::EOpConvUint64ToBool:
        zero = builder.makeUint64Constant(0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(Pspv::OpINotEqual, destType, operand, zero);
    case Pglslang::EOpConvDoubleToBool:
        zero = builder.makeDoubleConstant(0.0);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(Pspv::OpFOrdNotEqual, destType, operand, zero);
    case Pglslang::EOpConvFloat16ToBool:
        zero = builder.makeFloat16Constant(0.0F);
        zero = makeSmearedConstant(zero, vectorSize);
        return builder.createBinOp(Pspv::OpFOrdNotEqual, destType, operand, zero);
    case Pglslang::EOpConvBoolToDouble:
        convOp = Pspv::OpSelect;
        zero = builder.makeDoubleConstant(0.0);
        one  = builder.makeDoubleConstant(1.0);
        break;
    case Pglslang::EOpConvBoolToFloat16:
        convOp = Pspv::OpSelect;
        zero = builder.makeFloat16Constant(0.0F);
        one = builder.makeFloat16Constant(1.0F);
        break;
    case Pglslang::EOpConvBoolToInt8:
        zero = builder.makeInt8Constant(0);
        one  = builder.makeInt8Constant(1);
        convOp = Pspv::OpSelect;
        break;
    case Pglslang::EOpConvBoolToUint8:
        zero = builder.makeUint8Constant(0);
        one  = builder.makeUint8Constant(1);
        convOp = Pspv::OpSelect;
        break;
    case Pglslang::EOpConvBoolToInt16:
        zero = builder.makeInt16Constant(0);
        one  = builder.makeInt16Constant(1);
        convOp = Pspv::OpSelect;
        break;
    case Pglslang::EOpConvBoolToUint16:
        zero = builder.makeUint16Constant(0);
        one  = builder.makeUint16Constant(1);
        convOp = Pspv::OpSelect;
        break;
    case Pglslang::EOpConvDoubleToFloat:
    case Pglslang::EOpConvFloatToDouble:
    case Pglslang::EOpConvDoubleToFloat16:
    case Pglslang::EOpConvFloat16ToDouble:
    case Pglslang::EOpConvFloatToFloat16:
    case Pglslang::EOpConvFloat16ToFloat:
        convOp = Pspv::OpFConvert;
        if (builder.isMatrixType(destType))
            return createUnaryMatrixOperation(convOp, decorations, destType, operand, typeProxy);
        break;

    case Pglslang::EOpConvInt8ToInt16:
    case Pglslang::EOpConvInt8ToInt:
    case Pglslang::EOpConvInt8ToInt64:
    case Pglslang::EOpConvInt16ToInt8:
    case Pglslang::EOpConvInt16ToInt:
    case Pglslang::EOpConvInt16ToInt64:
    case Pglslang::EOpConvIntToInt8:
    case Pglslang::EOpConvIntToInt16:
    case Pglslang::EOpConvIntToInt64:
    case Pglslang::EOpConvInt64ToInt8:
    case Pglslang::EOpConvInt64ToInt16:
    case Pglslang::EOpConvInt64ToInt:
        convOp = Pspv::OpSConvert;
        break;

    case Pglslang::EOpConvUint8ToUint16:
    case Pglslang::EOpConvUint8ToUint:
    case Pglslang::EOpConvUint8ToUint64:
    case Pglslang::EOpConvUint16ToUint8:
    case Pglslang::EOpConvUint16ToUint:
    case Pglslang::EOpConvUint16ToUint64:
    case Pglslang::EOpConvUintToUint8:
    case Pglslang::EOpConvUintToUint16:
    case Pglslang::EOpConvUintToUint64:
    case Pglslang::EOpConvUint64ToUint8:
    case Pglslang::EOpConvUint64ToUint16:
    case Pglslang::EOpConvUint64ToUint:
        convOp = Pspv::OpUConvert;
        break;

    case Pglslang::EOpConvInt8ToUint16:
    case Pglslang::EOpConvInt8ToUint:
    case Pglslang::EOpConvInt8ToUint64:
    case Pglslang::EOpConvInt16ToUint8:
    case Pglslang::EOpConvInt16ToUint:
    case Pglslang::EOpConvInt16ToUint64:
    case Pglslang::EOpConvIntToUint8:
    case Pglslang::EOpConvIntToUint16:
    case Pglslang::EOpConvIntToUint64:
    case Pglslang::EOpConvInt64ToUint8:
    case Pglslang::EOpConvInt64ToUint16:
    case Pglslang::EOpConvInt64ToUint:
    case Pglslang::EOpConvUint8ToInt16:
    case Pglslang::EOpConvUint8ToInt:
    case Pglslang::EOpConvUint8ToInt64:
    case Pglslang::EOpConvUint16ToInt8:
    case Pglslang::EOpConvUint16ToInt:
    case Pglslang::EOpConvUint16ToInt64:
    case Pglslang::EOpConvUintToInt8:
    case Pglslang::EOpConvUintToInt16:
    case Pglslang::EOpConvUintToInt64:
    case Pglslang::EOpConvUint64ToInt8:
    case Pglslang::EOpConvUint64ToInt16:
    case Pglslang::EOpConvUint64ToInt:
        // OpSConvert/OpUConvert + OpBitCast
        operand = createIntWidthConversion(op, operand, vectorSize);

        if (builder.isInSpecConstCodeGenMode()) {
            // Build zero scalar or vector for OpIAdd.
            switch(op) {
            case Pglslang::EOpConvInt16ToUint8:
            case Pglslang::EOpConvIntToUint8:
            case Pglslang::EOpConvInt64ToUint8:
            case Pglslang::EOpConvUint16ToInt8:
            case Pglslang::EOpConvUintToInt8:
            case Pglslang::EOpConvUint64ToInt8:
                zero = builder.makeUint8Constant(0);
                break;
            case Pglslang::EOpConvInt8ToUint16:
            case Pglslang::EOpConvIntToUint16:
            case Pglslang::EOpConvInt64ToUint16:
            case Pglslang::EOpConvUint8ToInt16:
            case Pglslang::EOpConvUintToInt16:
            case Pglslang::EOpConvUint64ToInt16:
                zero = builder.makeUint16Constant(0);
                break;
            case Pglslang::EOpConvInt8ToUint:
            case Pglslang::EOpConvInt16ToUint:
            case Pglslang::EOpConvInt64ToUint:
            case Pglslang::EOpConvUint8ToInt:
            case Pglslang::EOpConvUint16ToInt:
            case Pglslang::EOpConvUint64ToInt:
                zero = builder.makeUintConstant(0);
                break;
            case Pglslang::EOpConvInt8ToUint64:
            case Pglslang::EOpConvInt16ToUint64:
            case Pglslang::EOpConvIntToUint64:
            case Pglslang::EOpConvUint8ToInt64:
            case Pglslang::EOpConvUint16ToInt64:
            case Pglslang::EOpConvUintToInt64:
                zero = builder.makeUint64Constant(0);
                break;
            default:
                assert(false && "Default missing");
                break;
            }
            zero = makeSmearedConstant(zero, vectorSize);
            // Use OpIAdd, instead of OpBitcast to do the conversion when
            // generating for OpSpecConstantOp instruction.
            return builder.createBinOp(Pspv::OpIAdd, destType, operand, zero);
        }
        // For normal run-time conversion instruction, use OpBitcast.
        convOp = Pspv::OpBitcast;
        break;
    case Pglslang::EOpConvUint64ToPtr:
        convOp = Pspv::OpConvertUToPtr;
        break;
    case Pglslang::EOpConvPtrToUint64:
        convOp = Pspv::OpConvertPtrToU;
        break;
    case Pglslang::EOpConvPtrToUvec2:
    case Pglslang::EOpConvUvec2ToPtr:
        if (builder.isVector(operand))
            builder.promoteIncorporatedExtension(Pspv::E_SPV_EXT_physical_storage_buffer,
                                                 Pspv::E_SPV_KHR_physical_storage_buffer, Pspv::Spv_1_5);
        convOp = Pspv::OpBitcast;
        break;
#endif

    default:
        break;
    }

    Pspv::Id result = 0;
    if (convOp == Pspv::OpNop)
        return result;

    if (convOp == Pspv::OpSelect) {
        zero = makeSmearedConstant(zero, vectorSize);
        one  = makeSmearedConstant(one, vectorSize);
        result = builder.createTriOp(convOp, destType, operand, one, zero);
    } else
        result = builder.createUnaryOp(convOp, destType, operand);

    result = builder.setPrecision(result, decorations.precision);
    decorations.addNonUniform(builder, result);
    return result;
}

Pspv::Id TGlslangToSpvTraverser::makeSmearedConstant(Pspv::Id constant, int vectorSize)
{
    if (vectorSize == 0)
        return constant;

    Pspv::Id vectorTypeId = builder.makeVectorType(builder.getTypeId(constant), vectorSize);
    std::vector<Pspv::Id> components;
    for (int c = 0; c < vectorSize; ++c)
        components.push_back(constant);
    return builder.makeCompositeConstant(vectorTypeId, components);
}

// For glslang ops that map to SPV atomic opCodes
Pspv::Id TGlslangToSpvTraverser::createAtomicOperation(Pglslang::TOperator op, Pspv::Decoration /*precision*/, Pspv::Id typeId, std::vector<Pspv::Id>& operands, Pglslang::TBasicType typeProxy, const Pspv::Builder::AccessChain::CoherentFlags &lvalueCoherentFlags)
{
    Pspv::Op opCode = Pspv::OpNop;

    switch (op) {
    case Pglslang::EOpAtomicAdd:
    case Pglslang::EOpImageAtomicAdd:
    case Pglslang::EOpAtomicCounterAdd:
        opCode = Pspv::OpAtomicIAdd;
        break;
    case Pglslang::EOpAtomicCounterSubtract:
        opCode = Pspv::OpAtomicISub;
        break;
    case Pglslang::EOpAtomicMin:
    case Pglslang::EOpImageAtomicMin:
    case Pglslang::EOpAtomicCounterMin:
        opCode = (typeProxy == Pglslang::EbtUint || typeProxy == Pglslang::EbtUint64) ? Pspv::OpAtomicUMin : Pspv::OpAtomicSMin;
        break;
    case Pglslang::EOpAtomicMax:
    case Pglslang::EOpImageAtomicMax:
    case Pglslang::EOpAtomicCounterMax:
        opCode = (typeProxy == Pglslang::EbtUint || typeProxy == Pglslang::EbtUint64) ? Pspv::OpAtomicUMax : Pspv::OpAtomicSMax;
        break;
    case Pglslang::EOpAtomicAnd:
    case Pglslang::EOpImageAtomicAnd:
    case Pglslang::EOpAtomicCounterAnd:
        opCode = Pspv::OpAtomicAnd;
        break;
    case Pglslang::EOpAtomicOr:
    case Pglslang::EOpImageAtomicOr:
    case Pglslang::EOpAtomicCounterOr:
        opCode = Pspv::OpAtomicOr;
        break;
    case Pglslang::EOpAtomicXor:
    case Pglslang::EOpImageAtomicXor:
    case Pglslang::EOpAtomicCounterXor:
        opCode = Pspv::OpAtomicXor;
        break;
    case Pglslang::EOpAtomicExchange:
    case Pglslang::EOpImageAtomicExchange:
    case Pglslang::EOpAtomicCounterExchange:
        opCode = Pspv::OpAtomicExchange;
        break;
    case Pglslang::EOpAtomicCompSwap:
    case Pglslang::EOpImageAtomicCompSwap:
    case Pglslang::EOpAtomicCounterCompSwap:
        opCode = Pspv::OpAtomicCompareExchange;
        break;
    case Pglslang::EOpAtomicCounterIncrement:
        opCode = Pspv::OpAtomicIIncrement;
        break;
    case Pglslang::EOpAtomicCounterDecrement:
        opCode = Pspv::OpAtomicIDecrement;
        break;
    case Pglslang::EOpAtomicCounter:
    case Pglslang::EOpImageAtomicLoad:
    case Pglslang::EOpAtomicLoad:
        opCode = Pspv::OpAtomicLoad;
        break;
    case Pglslang::EOpAtomicStore:
    case Pglslang::EOpImageAtomicStore:
        opCode = Pspv::OpAtomicStore;
        break;
    default:
        assert(0);
        break;
    }

    if (typeProxy == Pglslang::EbtInt64 || typeProxy == Pglslang::EbtUint64)
        builder.addCapability(Pspv::CapabilityInt64Atomics);

    // Sort out the operands
    //  - mapping from glslang -> SPV
    //  - there are extra SPV operands that are optional in glslang
    //  - compare-exchange swaps the value and comparator
    //  - compare-exchange has an extra memory semantics
    //  - EOpAtomicCounterDecrement needs a post decrement
    Pspv::Id pointerId = 0, compareId = 0, valueId = 0;
    // scope defaults to Device in the old model, QueueFamilyKHR in the new model
    Pspv::Id scopeId;
    if (glslangIntermediate->usingVulkanMemoryModel()) {
        scopeId = builder.makeUintConstant(Pspv::ScopeQueueFamilyKHR);
    } else {
        scopeId = builder.makeUintConstant(Pspv::ScopeDevice);
    }
    // semantics default to relaxed 
    Pspv::Id semanticsId = builder.makeUintConstant(lvalueCoherentFlags.isVolatile() && glslangIntermediate->usingVulkanMemoryModel() ?
                                                    Pspv::MemorySemanticsVolatileMask :
                                                    Pspv::MemorySemanticsMaskNone);
    Pspv::Id semanticsId2 = semanticsId;

    pointerId = operands[0];
    if (opCode == Pspv::OpAtomicIIncrement || opCode == Pspv::OpAtomicIDecrement) {
        // no additional operands
    } else if (opCode == Pspv::OpAtomicCompareExchange) {
        compareId = operands[1];
        valueId = operands[2];
        if (operands.size() > 3) {
            scopeId = operands[3];
            semanticsId = builder.makeUintConstant(builder.getConstantScalar(operands[4]) | builder.getConstantScalar(operands[5]));
            semanticsId2 = builder.makeUintConstant(builder.getConstantScalar(operands[6]) | builder.getConstantScalar(operands[7]));
        }
    } else if (opCode == Pspv::OpAtomicLoad) {
        if (operands.size() > 1) {
            scopeId = operands[1];
            semanticsId = builder.makeUintConstant(builder.getConstantScalar(operands[2]) | builder.getConstantScalar(operands[3]));
        }
    } else {
        // atomic store or RMW
        valueId = operands[1];
        if (operands.size() > 2) {
            scopeId = operands[2];
            semanticsId = builder.makeUintConstant(builder.getConstantScalar(operands[3]) | builder.getConstantScalar(operands[4]));
        }
    }

    // Check for capabilities
    unsigned semanticsImmediate = builder.getConstantScalar(semanticsId) | builder.getConstantScalar(semanticsId2);
    if (semanticsImmediate & (Pspv::MemorySemanticsMakeAvailableKHRMask |
                              Pspv::MemorySemanticsMakeVisibleKHRMask |
                              Pspv::MemorySemanticsOutputMemoryKHRMask |
                              Pspv::MemorySemanticsVolatileMask)) {
        builder.addCapability(Pspv::CapabilityVulkanMemoryModelKHR);
    }

    if (glslangIntermediate->usingVulkanMemoryModel() && builder.getConstantScalar(scopeId) == Pspv::ScopeDevice) {
        builder.addCapability(Pspv::CapabilityVulkanMemoryModelDeviceScopeKHR);
    }

    std::vector<Pspv::Id> spvAtomicOperands;  // hold the spv operands
    spvAtomicOperands.push_back(pointerId);
    spvAtomicOperands.push_back(scopeId);
    spvAtomicOperands.push_back(semanticsId);
    if (opCode == Pspv::OpAtomicCompareExchange) {
        spvAtomicOperands.push_back(semanticsId2);
        spvAtomicOperands.push_back(valueId);
        spvAtomicOperands.push_back(compareId);
    } else if (opCode != Pspv::OpAtomicLoad && opCode != Pspv::OpAtomicIIncrement && opCode != Pspv::OpAtomicIDecrement) {
        spvAtomicOperands.push_back(valueId);
    }

    if (opCode == Pspv::OpAtomicStore) {
        builder.createNoResultOp(opCode, spvAtomicOperands);
        return 0;
    } else {
        Pspv::Id resultId = builder.createOp(opCode, typeId, spvAtomicOperands);

        // GLSL and HLSL atomic-counter decrement return post-decrement value,
        // while SPIR-V returns pre-decrement value. Translate between these semantics.
        if (op == Pglslang::EOpAtomicCounterDecrement)
            resultId = builder.createBinOp(Pspv::OpISub, typeId, resultId, builder.makeIntConstant(1));

        return resultId;
    }
}

// Create group invocation operations.
Pspv::Id TGlslangToSpvTraverser::createInvocationsOperation(Pglslang::TOperator op, Pspv::Id typeId, std::vector<Pspv::Id>& operands, Pglslang::TBasicType typeProxy)
{
    bool isUnsigned = isTypeUnsignedInt(typeProxy);
    bool isFloat = isTypeFloat(typeProxy);

    Pspv::Op opCode = Pspv::OpNop;
    std::vector<Pspv::IdImmediate> spvGroupOperands;
    Pspv::GroupOperation groupOperation = Pspv::GroupOperationMax;

    if (op == Pglslang::EOpBallot || op == Pglslang::EOpReadFirstInvocation ||
        op == Pglslang::EOpReadInvocation) {
        builder.addExtension(Pspv::E_SPV_KHR_shader_ballot);
        builder.addCapability(Pspv::CapabilitySubgroupBallotKHR);
    } else if (op == Pglslang::EOpAnyInvocation ||
        op == Pglslang::EOpAllInvocations ||
        op == Pglslang::EOpAllInvocationsEqual) {
        builder.addExtension(Pspv::E_SPV_KHR_subgroup_vote);
        builder.addCapability(Pspv::CapabilitySubgroupVoteKHR);
    } else {
        builder.addCapability(Pspv::CapabilityGroups);
        if (op == Pglslang::EOpMinInvocationsNonUniform ||
            op == Pglslang::EOpMaxInvocationsNonUniform ||
            op == Pglslang::EOpAddInvocationsNonUniform ||
            op == Pglslang::EOpMinInvocationsInclusiveScanNonUniform ||
            op == Pglslang::EOpMaxInvocationsInclusiveScanNonUniform ||
            op == Pglslang::EOpAddInvocationsInclusiveScanNonUniform ||
            op == Pglslang::EOpMinInvocationsExclusiveScanNonUniform ||
            op == Pglslang::EOpMaxInvocationsExclusiveScanNonUniform ||
            op == Pglslang::EOpAddInvocationsExclusiveScanNonUniform)
            builder.addExtension(Pspv::E_SPV_AMD_shader_ballot);

        switch (op) {
        case Pglslang::EOpMinInvocations:
        case Pglslang::EOpMaxInvocations:
        case Pglslang::EOpAddInvocations:
        case Pglslang::EOpMinInvocationsNonUniform:
        case Pglslang::EOpMaxInvocationsNonUniform:
        case Pglslang::EOpAddInvocationsNonUniform:
            groupOperation = Pspv::GroupOperationReduce;
            break;
        case Pglslang::EOpMinInvocationsInclusiveScan:
        case Pglslang::EOpMaxInvocationsInclusiveScan:
        case Pglslang::EOpAddInvocationsInclusiveScan:
        case Pglslang::EOpMinInvocationsInclusiveScanNonUniform:
        case Pglslang::EOpMaxInvocationsInclusiveScanNonUniform:
        case Pglslang::EOpAddInvocationsInclusiveScanNonUniform:
            groupOperation = Pspv::GroupOperationInclusiveScan;
            break;
        case Pglslang::EOpMinInvocationsExclusiveScan:
        case Pglslang::EOpMaxInvocationsExclusiveScan:
        case Pglslang::EOpAddInvocationsExclusiveScan:
        case Pglslang::EOpMinInvocationsExclusiveScanNonUniform:
        case Pglslang::EOpMaxInvocationsExclusiveScanNonUniform:
        case Pglslang::EOpAddInvocationsExclusiveScanNonUniform:
            groupOperation = Pspv::GroupOperationExclusiveScan;
            break;
        default:
            break;
        }
        Pspv::IdImmediate scope = { true, builder.makeUintConstant(Pspv::ScopeSubgroup) };
        spvGroupOperands.push_back(scope);
        if (groupOperation != Pspv::GroupOperationMax) {
            Pspv::IdImmediate groupOp = { false, (unsigned)groupOperation };
            spvGroupOperands.push_back(groupOp);
        }
    }

    for (auto opIt = operands.begin(); opIt != operands.end(); ++opIt) {
        Pspv::IdImmediate op = { true, *opIt };
        spvGroupOperands.push_back(op);
    }

    switch (op) {
    case Pglslang::EOpAnyInvocation:
        opCode = Pspv::OpSubgroupAnyKHR;
        break;
    case Pglslang::EOpAllInvocations:
        opCode = Pspv::OpSubgroupAllKHR;
        break;
    case Pglslang::EOpAllInvocationsEqual:
        opCode = Pspv::OpSubgroupAllEqualKHR;
        break;
    case Pglslang::EOpReadInvocation:
        opCode = Pspv::OpSubgroupReadInvocationKHR;
        if (builder.isVectorType(typeId))
            return CreateInvocationsVectorOperation(opCode, groupOperation, typeId, operands);
        break;
    case Pglslang::EOpReadFirstInvocation:
        opCode = Pspv::OpSubgroupFirstInvocationKHR;
        break;
    case Pglslang::EOpBallot:
    {
        // NOTE: According to the spec, the result type of "OpSubgroupBallotKHR" must be a 4 component vector of 32
        // bit integer types. The GLSL built-in function "ballotARB()" assumes the maximum number of invocations in
        // a subgroup is 64. Thus, we have to convert uvec4.xy to uint64_t as follow:
        //
        //     result = Bitcast(SubgroupBallotKHR(Predicate).xy)
        //
        Pspv::Id uintType  = builder.makeUintType(32);
        Pspv::Id uvec4Type = builder.makeVectorType(uintType, 4);
        Pspv::Id result = builder.createOp(Pspv::OpSubgroupBallotKHR, uvec4Type, spvGroupOperands);

        std::vector<Pspv::Id> components;
        components.push_back(builder.createCompositeExtract(result, uintType, 0));
        components.push_back(builder.createCompositeExtract(result, uintType, 1));

        Pspv::Id uvec2Type = builder.makeVectorType(uintType, 2);
        return builder.createUnaryOp(Pspv::OpBitcast, typeId,
                                     builder.createCompositeConstruct(uvec2Type, components));
    }

    case Pglslang::EOpMinInvocations:
    case Pglslang::EOpMaxInvocations:
    case Pglslang::EOpAddInvocations:
    case Pglslang::EOpMinInvocationsInclusiveScan:
    case Pglslang::EOpMaxInvocationsInclusiveScan:
    case Pglslang::EOpAddInvocationsInclusiveScan:
    case Pglslang::EOpMinInvocationsExclusiveScan:
    case Pglslang::EOpMaxInvocationsExclusiveScan:
    case Pglslang::EOpAddInvocationsExclusiveScan:
        if (op == Pglslang::EOpMinInvocations ||
            op == Pglslang::EOpMinInvocationsInclusiveScan ||
            op == Pglslang::EOpMinInvocationsExclusiveScan) {
            if (isFloat)
                opCode = Pspv::OpGroupFMin;
            else {
                if (isUnsigned)
                    opCode = Pspv::OpGroupUMin;
                else
                    opCode = Pspv::OpGroupSMin;
            }
        } else if (op == Pglslang::EOpMaxInvocations ||
                   op == Pglslang::EOpMaxInvocationsInclusiveScan ||
                   op == Pglslang::EOpMaxInvocationsExclusiveScan) {
            if (isFloat)
                opCode = Pspv::OpGroupFMax;
            else {
                if (isUnsigned)
                    opCode = Pspv::OpGroupUMax;
                else
                    opCode = Pspv::OpGroupSMax;
            }
        } else {
            if (isFloat)
                opCode = Pspv::OpGroupFAdd;
            else
                opCode = Pspv::OpGroupIAdd;
        }

        if (builder.isVectorType(typeId))
            return CreateInvocationsVectorOperation(opCode, groupOperation, typeId, operands);

        break;
    case Pglslang::EOpMinInvocationsNonUniform:
    case Pglslang::EOpMaxInvocationsNonUniform:
    case Pglslang::EOpAddInvocationsNonUniform:
    case Pglslang::EOpMinInvocationsInclusiveScanNonUniform:
    case Pglslang::EOpMaxInvocationsInclusiveScanNonUniform:
    case Pglslang::EOpAddInvocationsInclusiveScanNonUniform:
    case Pglslang::EOpMinInvocationsExclusiveScanNonUniform:
    case Pglslang::EOpMaxInvocationsExclusiveScanNonUniform:
    case Pglslang::EOpAddInvocationsExclusiveScanNonUniform:
        if (op == Pglslang::EOpMinInvocationsNonUniform ||
            op == Pglslang::EOpMinInvocationsInclusiveScanNonUniform ||
            op == Pglslang::EOpMinInvocationsExclusiveScanNonUniform) {
            if (isFloat)
                opCode = Pspv::OpGroupFMinNonUniformAMD;
            else {
                if (isUnsigned)
                    opCode = Pspv::OpGroupUMinNonUniformAMD;
                else
                    opCode = Pspv::OpGroupSMinNonUniformAMD;
            }
        }
        else if (op == Pglslang::EOpMaxInvocationsNonUniform ||
                 op == Pglslang::EOpMaxInvocationsInclusiveScanNonUniform ||
                 op == Pglslang::EOpMaxInvocationsExclusiveScanNonUniform) {
            if (isFloat)
                opCode = Pspv::OpGroupFMaxNonUniformAMD;
            else {
                if (isUnsigned)
                    opCode = Pspv::OpGroupUMaxNonUniformAMD;
                else
                    opCode = Pspv::OpGroupSMaxNonUniformAMD;
            }
        }
        else {
            if (isFloat)
                opCode = Pspv::OpGroupFAddNonUniformAMD;
            else
                opCode = Pspv::OpGroupIAddNonUniformAMD;
        }

        if (builder.isVectorType(typeId))
            return CreateInvocationsVectorOperation(opCode, groupOperation, typeId, operands);

        break;
    default:
        logger->missingFunctionality("invocation operation");
        return Pspv::NoResult;
    }

    assert(opCode != Pspv::OpNop);
    return builder.createOp(opCode, typeId, spvGroupOperands);
}

// Create group invocation operations on a vector
Pspv::Id TGlslangToSpvTraverser::CreateInvocationsVectorOperation(Pspv::Op op, Pspv::GroupOperation groupOperation,
    Pspv::Id typeId, std::vector<Pspv::Id>& operands)
{
    assert(op == Pspv::OpGroupFMin || op == Pspv::OpGroupUMin || op == Pspv::OpGroupSMin ||
           op == Pspv::OpGroupFMax || op == Pspv::OpGroupUMax || op == Pspv::OpGroupSMax ||
           op == Pspv::OpGroupFAdd || op == Pspv::OpGroupIAdd || op == Pspv::OpGroupBroadcast ||
           op == Pspv::OpSubgroupReadInvocationKHR ||
           op == Pspv::OpGroupFMinNonUniformAMD || op == Pspv::OpGroupUMinNonUniformAMD || op == Pspv::OpGroupSMinNonUniformAMD ||
           op == Pspv::OpGroupFMaxNonUniformAMD || op == Pspv::OpGroupUMaxNonUniformAMD || op == Pspv::OpGroupSMaxNonUniformAMD ||
           op == Pspv::OpGroupFAddNonUniformAMD || op == Pspv::OpGroupIAddNonUniformAMD);

    // Handle group invocation operations scalar by scalar.
    // The result type is the same type as the original type.
    // The algorithm is to:
    //   - break the vector into scalars
    //   - apply the operation to each scalar
    //   - make a vector out the scalar results

    // get the types sorted out
    int numComponents = builder.getNumComponents(operands[0]);
    Pspv::Id scalarType = builder.getScalarTypeId(builder.getTypeId(operands[0]));
    std::vector<Pspv::Id> results;

    // do each scalar op
    for (int comp = 0; comp < numComponents; ++comp) {
        std::vector<unsigned int> indexes;
        indexes.push_back(comp);
        Pspv::IdImmediate scalar = { true, builder.createCompositeExtract(operands[0], scalarType, indexes) };
        std::vector<Pspv::IdImmediate> spvGroupOperands;
        if (op == Pspv::OpSubgroupReadInvocationKHR) {
            spvGroupOperands.push_back(scalar);
            Pspv::IdImmediate operand = { true, operands[1] };
            spvGroupOperands.push_back(operand);
        } else if (op == Pspv::OpGroupBroadcast) {
            Pspv::IdImmediate scope = { true, builder.makeUintConstant(Pspv::ScopeSubgroup) };
            spvGroupOperands.push_back(scope);
            spvGroupOperands.push_back(scalar);
            Pspv::IdImmediate operand = { true, operands[1] };
            spvGroupOperands.push_back(operand);
        } else {
            Pspv::IdImmediate scope = { true, builder.makeUintConstant(Pspv::ScopeSubgroup) };
            spvGroupOperands.push_back(scope);
            Pspv::IdImmediate groupOp = { false, (unsigned)groupOperation };
            spvGroupOperands.push_back(groupOp);
            spvGroupOperands.push_back(scalar);
        }

        results.push_back(builder.createOp(op, scalarType, spvGroupOperands));
    }

    // put the pieces together
    return builder.createCompositeConstruct(typeId, results);
}

// Create subgroup invocation operations.
Pspv::Id TGlslangToSpvTraverser::createSubgroupOperation(Pglslang::TOperator op, Pspv::Id typeId,
    std::vector<Pspv::Id>& operands, Pglslang::TBasicType typeProxy)
{
    // Add the required capabilities.
    switch (op) {
    case Pglslang::EOpSubgroupElect:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        break;
    case Pglslang::EOpSubgroupAll:
    case Pglslang::EOpSubgroupAny:
    case Pglslang::EOpSubgroupAllEqual:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformVote);
        break;
    case Pglslang::EOpSubgroupBroadcast:
    case Pglslang::EOpSubgroupBroadcastFirst:
    case Pglslang::EOpSubgroupBallot:
    case Pglslang::EOpSubgroupInverseBallot:
    case Pglslang::EOpSubgroupBallotBitExtract:
    case Pglslang::EOpSubgroupBallotBitCount:
    case Pglslang::EOpSubgroupBallotInclusiveBitCount:
    case Pglslang::EOpSubgroupBallotExclusiveBitCount:
    case Pglslang::EOpSubgroupBallotFindLSB:
    case Pglslang::EOpSubgroupBallotFindMSB:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformBallot);
        break;
    case Pglslang::EOpSubgroupShuffle:
    case Pglslang::EOpSubgroupShuffleXor:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformShuffle);
        break;
    case Pglslang::EOpSubgroupShuffleUp:
    case Pglslang::EOpSubgroupShuffleDown:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformShuffleRelative);
        break;
    case Pglslang::EOpSubgroupAdd:
    case Pglslang::EOpSubgroupMul:
    case Pglslang::EOpSubgroupMin:
    case Pglslang::EOpSubgroupMax:
    case Pglslang::EOpSubgroupAnd:
    case Pglslang::EOpSubgroupOr:
    case Pglslang::EOpSubgroupXor:
    case Pglslang::EOpSubgroupInclusiveAdd:
    case Pglslang::EOpSubgroupInclusiveMul:
    case Pglslang::EOpSubgroupInclusiveMin:
    case Pglslang::EOpSubgroupInclusiveMax:
    case Pglslang::EOpSubgroupInclusiveAnd:
    case Pglslang::EOpSubgroupInclusiveOr:
    case Pglslang::EOpSubgroupInclusiveXor:
    case Pglslang::EOpSubgroupExclusiveAdd:
    case Pglslang::EOpSubgroupExclusiveMul:
    case Pglslang::EOpSubgroupExclusiveMin:
    case Pglslang::EOpSubgroupExclusiveMax:
    case Pglslang::EOpSubgroupExclusiveAnd:
    case Pglslang::EOpSubgroupExclusiveOr:
    case Pglslang::EOpSubgroupExclusiveXor:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformArithmetic);
        break;
    case Pglslang::EOpSubgroupClusteredAdd:
    case Pglslang::EOpSubgroupClusteredMul:
    case Pglslang::EOpSubgroupClusteredMin:
    case Pglslang::EOpSubgroupClusteredMax:
    case Pglslang::EOpSubgroupClusteredAnd:
    case Pglslang::EOpSubgroupClusteredOr:
    case Pglslang::EOpSubgroupClusteredXor:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformClustered);
        break;
    case Pglslang::EOpSubgroupQuadBroadcast:
    case Pglslang::EOpSubgroupQuadSwapHorizontal:
    case Pglslang::EOpSubgroupQuadSwapVertical:
    case Pglslang::EOpSubgroupQuadSwapDiagonal:
        builder.addCapability(Pspv::CapabilityGroupNonUniform);
        builder.addCapability(Pspv::CapabilityGroupNonUniformQuad);
        break;
    case Pglslang::EOpSubgroupPartitionedAdd:
    case Pglslang::EOpSubgroupPartitionedMul:
    case Pglslang::EOpSubgroupPartitionedMin:
    case Pglslang::EOpSubgroupPartitionedMax:
    case Pglslang::EOpSubgroupPartitionedAnd:
    case Pglslang::EOpSubgroupPartitionedOr:
    case Pglslang::EOpSubgroupPartitionedXor:
    case Pglslang::EOpSubgroupPartitionedInclusiveAdd:
    case Pglslang::EOpSubgroupPartitionedInclusiveMul:
    case Pglslang::EOpSubgroupPartitionedInclusiveMin:
    case Pglslang::EOpSubgroupPartitionedInclusiveMax:
    case Pglslang::EOpSubgroupPartitionedInclusiveAnd:
    case Pglslang::EOpSubgroupPartitionedInclusiveOr:
    case Pglslang::EOpSubgroupPartitionedInclusiveXor:
    case Pglslang::EOpSubgroupPartitionedExclusiveAdd:
    case Pglslang::EOpSubgroupPartitionedExclusiveMul:
    case Pglslang::EOpSubgroupPartitionedExclusiveMin:
    case Pglslang::EOpSubgroupPartitionedExclusiveMax:
    case Pglslang::EOpSubgroupPartitionedExclusiveAnd:
    case Pglslang::EOpSubgroupPartitionedExclusiveOr:
    case Pglslang::EOpSubgroupPartitionedExclusiveXor:
        builder.addExtension(Pspv::E_SPV_NV_shader_subgroup_partitioned);
        builder.addCapability(Pspv::CapabilityGroupNonUniformPartitionedNV);
        break;
    default: assert(0 && "Unhandled subgroup operation!");
    }


    const bool isUnsigned = isTypeUnsignedInt(typeProxy);
    const bool isFloat = isTypeFloat(typeProxy);
    const bool isBool = typeProxy == Pglslang::EbtBool;

    Pspv::Op opCode = Pspv::OpNop;

    // Figure out which opcode to use.
    switch (op) {
    case Pglslang::EOpSubgroupElect:                   opCode = Pspv::OpGroupNonUniformElect; break;
    case Pglslang::EOpSubgroupAll:                     opCode = Pspv::OpGroupNonUniformAll; break;
    case Pglslang::EOpSubgroupAny:                     opCode = Pspv::OpGroupNonUniformAny; break;
    case Pglslang::EOpSubgroupAllEqual:                opCode = Pspv::OpGroupNonUniformAllEqual; break;
    case Pglslang::EOpSubgroupBroadcast:               opCode = Pspv::OpGroupNonUniformBroadcast; break;
    case Pglslang::EOpSubgroupBroadcastFirst:          opCode = Pspv::OpGroupNonUniformBroadcastFirst; break;
    case Pglslang::EOpSubgroupBallot:                  opCode = Pspv::OpGroupNonUniformBallot; break;
    case Pglslang::EOpSubgroupInverseBallot:           opCode = Pspv::OpGroupNonUniformInverseBallot; break;
    case Pglslang::EOpSubgroupBallotBitExtract:        opCode = Pspv::OpGroupNonUniformBallotBitExtract; break;
    case Pglslang::EOpSubgroupBallotBitCount:
    case Pglslang::EOpSubgroupBallotInclusiveBitCount:
    case Pglslang::EOpSubgroupBallotExclusiveBitCount: opCode = Pspv::OpGroupNonUniformBallotBitCount; break;
    case Pglslang::EOpSubgroupBallotFindLSB:           opCode = Pspv::OpGroupNonUniformBallotFindLSB; break;
    case Pglslang::EOpSubgroupBallotFindMSB:           opCode = Pspv::OpGroupNonUniformBallotFindMSB; break;
    case Pglslang::EOpSubgroupShuffle:                 opCode = Pspv::OpGroupNonUniformShuffle; break;
    case Pglslang::EOpSubgroupShuffleXor:              opCode = Pspv::OpGroupNonUniformShuffleXor; break;
    case Pglslang::EOpSubgroupShuffleUp:               opCode = Pspv::OpGroupNonUniformShuffleUp; break;
    case Pglslang::EOpSubgroupShuffleDown:             opCode = Pspv::OpGroupNonUniformShuffleDown; break;
    case Pglslang::EOpSubgroupAdd:
    case Pglslang::EOpSubgroupInclusiveAdd:
    case Pglslang::EOpSubgroupExclusiveAdd:
    case Pglslang::EOpSubgroupClusteredAdd:
    case Pglslang::EOpSubgroupPartitionedAdd:
    case Pglslang::EOpSubgroupPartitionedInclusiveAdd:
    case Pglslang::EOpSubgroupPartitionedExclusiveAdd:
        if (isFloat) {
            opCode = Pspv::OpGroupNonUniformFAdd;
        } else {
            opCode = Pspv::OpGroupNonUniformIAdd;
        }
        break;
    case Pglslang::EOpSubgroupMul:
    case Pglslang::EOpSubgroupInclusiveMul:
    case Pglslang::EOpSubgroupExclusiveMul:
    case Pglslang::EOpSubgroupClusteredMul:
    case Pglslang::EOpSubgroupPartitionedMul:
    case Pglslang::EOpSubgroupPartitionedInclusiveMul:
    case Pglslang::EOpSubgroupPartitionedExclusiveMul:
        if (isFloat) {
            opCode = Pspv::OpGroupNonUniformFMul;
        } else {
            opCode = Pspv::OpGroupNonUniformIMul;
        }
        break;
    case Pglslang::EOpSubgroupMin:
    case Pglslang::EOpSubgroupInclusiveMin:
    case Pglslang::EOpSubgroupExclusiveMin:
    case Pglslang::EOpSubgroupClusteredMin:
    case Pglslang::EOpSubgroupPartitionedMin:
    case Pglslang::EOpSubgroupPartitionedInclusiveMin:
    case Pglslang::EOpSubgroupPartitionedExclusiveMin:
        if (isFloat) {
            opCode = Pspv::OpGroupNonUniformFMin;
        } else if (isUnsigned) {
            opCode = Pspv::OpGroupNonUniformUMin;
        } else {
            opCode = Pspv::OpGroupNonUniformSMin;
        }
        break;
    case Pglslang::EOpSubgroupMax:
    case Pglslang::EOpSubgroupInclusiveMax:
    case Pglslang::EOpSubgroupExclusiveMax:
    case Pglslang::EOpSubgroupClusteredMax:
    case Pglslang::EOpSubgroupPartitionedMax:
    case Pglslang::EOpSubgroupPartitionedInclusiveMax:
    case Pglslang::EOpSubgroupPartitionedExclusiveMax:
        if (isFloat) {
            opCode = Pspv::OpGroupNonUniformFMax;
        } else if (isUnsigned) {
            opCode = Pspv::OpGroupNonUniformUMax;
        } else {
            opCode = Pspv::OpGroupNonUniformSMax;
        }
        break;
    case Pglslang::EOpSubgroupAnd:
    case Pglslang::EOpSubgroupInclusiveAnd:
    case Pglslang::EOpSubgroupExclusiveAnd:
    case Pglslang::EOpSubgroupClusteredAnd:
    case Pglslang::EOpSubgroupPartitionedAnd:
    case Pglslang::EOpSubgroupPartitionedInclusiveAnd:
    case Pglslang::EOpSubgroupPartitionedExclusiveAnd:
        if (isBool) {
            opCode = Pspv::OpGroupNonUniformLogicalAnd;
        } else {
            opCode = Pspv::OpGroupNonUniformBitwiseAnd;
        }
        break;
    case Pglslang::EOpSubgroupOr:
    case Pglslang::EOpSubgroupInclusiveOr:
    case Pglslang::EOpSubgroupExclusiveOr:
    case Pglslang::EOpSubgroupClusteredOr:
    case Pglslang::EOpSubgroupPartitionedOr:
    case Pglslang::EOpSubgroupPartitionedInclusiveOr:
    case Pglslang::EOpSubgroupPartitionedExclusiveOr:
        if (isBool) {
            opCode = Pspv::OpGroupNonUniformLogicalOr;
        } else {
            opCode = Pspv::OpGroupNonUniformBitwiseOr;
        }
        break;
    case Pglslang::EOpSubgroupXor:
    case Pglslang::EOpSubgroupInclusiveXor:
    case Pglslang::EOpSubgroupExclusiveXor:
    case Pglslang::EOpSubgroupClusteredXor:
    case Pglslang::EOpSubgroupPartitionedXor:
    case Pglslang::EOpSubgroupPartitionedInclusiveXor:
    case Pglslang::EOpSubgroupPartitionedExclusiveXor:
        if (isBool) {
            opCode = Pspv::OpGroupNonUniformLogicalXor;
        } else {
            opCode = Pspv::OpGroupNonUniformBitwiseXor;
        }
        break;
    case Pglslang::EOpSubgroupQuadBroadcast:      opCode = Pspv::OpGroupNonUniformQuadBroadcast; break;
    case Pglslang::EOpSubgroupQuadSwapHorizontal:
    case Pglslang::EOpSubgroupQuadSwapVertical:
    case Pglslang::EOpSubgroupQuadSwapDiagonal:   opCode = Pspv::OpGroupNonUniformQuadSwap; break;
    default: assert(0 && "Unhandled subgroup operation!");
    }

    // get the right Group Operation
    Pspv::GroupOperation groupOperation = Pspv::GroupOperationMax;
    switch (op) {
    default:
        break;
    case Pglslang::EOpSubgroupBallotBitCount:
    case Pglslang::EOpSubgroupAdd:
    case Pglslang::EOpSubgroupMul:
    case Pglslang::EOpSubgroupMin:
    case Pglslang::EOpSubgroupMax:
    case Pglslang::EOpSubgroupAnd:
    case Pglslang::EOpSubgroupOr:
    case Pglslang::EOpSubgroupXor:
        groupOperation = Pspv::GroupOperationReduce;
        break;
    case Pglslang::EOpSubgroupBallotInclusiveBitCount:
    case Pglslang::EOpSubgroupInclusiveAdd:
    case Pglslang::EOpSubgroupInclusiveMul:
    case Pglslang::EOpSubgroupInclusiveMin:
    case Pglslang::EOpSubgroupInclusiveMax:
    case Pglslang::EOpSubgroupInclusiveAnd:
    case Pglslang::EOpSubgroupInclusiveOr:
    case Pglslang::EOpSubgroupInclusiveXor:
        groupOperation = Pspv::GroupOperationInclusiveScan;
        break;
    case Pglslang::EOpSubgroupBallotExclusiveBitCount:
    case Pglslang::EOpSubgroupExclusiveAdd:
    case Pglslang::EOpSubgroupExclusiveMul:
    case Pglslang::EOpSubgroupExclusiveMin:
    case Pglslang::EOpSubgroupExclusiveMax:
    case Pglslang::EOpSubgroupExclusiveAnd:
    case Pglslang::EOpSubgroupExclusiveOr:
    case Pglslang::EOpSubgroupExclusiveXor:
        groupOperation = Pspv::GroupOperationExclusiveScan;
        break;
    case Pglslang::EOpSubgroupClusteredAdd:
    case Pglslang::EOpSubgroupClusteredMul:
    case Pglslang::EOpSubgroupClusteredMin:
    case Pglslang::EOpSubgroupClusteredMax:
    case Pglslang::EOpSubgroupClusteredAnd:
    case Pglslang::EOpSubgroupClusteredOr:
    case Pglslang::EOpSubgroupClusteredXor:
        groupOperation = Pspv::GroupOperationClusteredReduce;
        break;
    case Pglslang::EOpSubgroupPartitionedAdd:
    case Pglslang::EOpSubgroupPartitionedMul:
    case Pglslang::EOpSubgroupPartitionedMin:
    case Pglslang::EOpSubgroupPartitionedMax:
    case Pglslang::EOpSubgroupPartitionedAnd:
    case Pglslang::EOpSubgroupPartitionedOr:
    case Pglslang::EOpSubgroupPartitionedXor:
        groupOperation = Pspv::GroupOperationPartitionedReduceNV;
        break;
    case Pglslang::EOpSubgroupPartitionedInclusiveAdd:
    case Pglslang::EOpSubgroupPartitionedInclusiveMul:
    case Pglslang::EOpSubgroupPartitionedInclusiveMin:
    case Pglslang::EOpSubgroupPartitionedInclusiveMax:
    case Pglslang::EOpSubgroupPartitionedInclusiveAnd:
    case Pglslang::EOpSubgroupPartitionedInclusiveOr:
    case Pglslang::EOpSubgroupPartitionedInclusiveXor:
        groupOperation = Pspv::GroupOperationPartitionedInclusiveScanNV;
        break;
    case Pglslang::EOpSubgroupPartitionedExclusiveAdd:
    case Pglslang::EOpSubgroupPartitionedExclusiveMul:
    case Pglslang::EOpSubgroupPartitionedExclusiveMin:
    case Pglslang::EOpSubgroupPartitionedExclusiveMax:
    case Pglslang::EOpSubgroupPartitionedExclusiveAnd:
    case Pglslang::EOpSubgroupPartitionedExclusiveOr:
    case Pglslang::EOpSubgroupPartitionedExclusiveXor:
        groupOperation = Pspv::GroupOperationPartitionedExclusiveScanNV;
        break;
    }

    // build the instruction
    std::vector<Pspv::IdImmediate> spvGroupOperands;

    // Every operation begins with the Execution Scope operand.
    Pspv::IdImmediate executionScope = { true, builder.makeUintConstant(Pspv::ScopeSubgroup) };
    spvGroupOperands.push_back(executionScope);

    // Next, for all operations that use a Group Operation, push that as an operand.
    if (groupOperation != Pspv::GroupOperationMax) {
        Pspv::IdImmediate groupOperand = { false, (unsigned)groupOperation };
        spvGroupOperands.push_back(groupOperand);
    }

    // Push back the operands next.
    for (auto opIt = operands.cbegin(); opIt != operands.cend(); ++opIt) {
        Pspv::IdImmediate operand = { true, *opIt };
        spvGroupOperands.push_back(operand);
    }

    // Some opcodes have additional operands.
    Pspv::Id directionId = Pspv::NoResult;
    switch (op) {
    default: break;
    case Pglslang::EOpSubgroupQuadSwapHorizontal: directionId = builder.makeUintConstant(0); break;
    case Pglslang::EOpSubgroupQuadSwapVertical:   directionId = builder.makeUintConstant(1); break;
    case Pglslang::EOpSubgroupQuadSwapDiagonal:   directionId = builder.makeUintConstant(2); break;
    }
    if (directionId != Pspv::NoResult) {
        Pspv::IdImmediate direction = { true, directionId };
        spvGroupOperands.push_back(direction);
    }

    return builder.createOp(opCode, typeId, spvGroupOperands);
}

Pspv::Id TGlslangToSpvTraverser::createMiscOperation(Pglslang::TOperator op, Pspv::Decoration precision, Pspv::Id typeId, std::vector<Pspv::Id>& operands, Pglslang::TBasicType typeProxy)
{
    bool isUnsigned = isTypeUnsignedInt(typeProxy);
    bool isFloat = isTypeFloat(typeProxy);

    Pspv::Op opCode = Pspv::OpNop;
    int extBuiltins = -1;
    int libCall = -1;
    size_t consumedOperands = operands.size();
    Pspv::Id typeId0 = 0;
    if (consumedOperands > 0)
        typeId0 = builder.getTypeId(operands[0]);
    Pspv::Id typeId1 = 0;
    if (consumedOperands > 1)
        typeId1 = builder.getTypeId(operands[1]);
    Pspv::Id frexpIntType = 0;

    switch (op) {
    case Pglslang::EOpMin:
        if (isFloat)
            libCall = nanMinMaxClamp ? Pspv::GLSLstd450NMin : Pspv::GLSLstd450FMin;
        else if (isUnsigned)
            libCall = Pspv::GLSLstd450UMin;
        else
            libCall = Pspv::GLSLstd450SMin;
        builder.promoteScalar(precision, operands.front(), operands.back());
        break;
    case Pglslang::EOpModf:
        libCall = Pspv::GLSLstd450Modf;
        break;
    case Pglslang::EOpMax:
        if (isFloat)
            libCall = nanMinMaxClamp ? Pspv::GLSLstd450NMax : Pspv::GLSLstd450FMax;
        else if (isUnsigned)
            libCall = Pspv::GLSLstd450UMax;
        else
            libCall = Pspv::GLSLstd450SMax;
        builder.promoteScalar(precision, operands.front(), operands.back());
        break;
    case Pglslang::EOpPow:
        libCall = Pspv::GLSLstd450Pow;
        break;
    case Pglslang::EOpDot:
        opCode = Pspv::OpDot;
        break;
    case Pglslang::EOpAtan:
        libCall = Pspv::GLSLstd450Atan2;
        break;

    case Pglslang::EOpClamp:
        if (isFloat)
            libCall = nanMinMaxClamp ? Pspv::GLSLstd450NClamp : Pspv::GLSLstd450FClamp;
        else if (isUnsigned)
            libCall = Pspv::GLSLstd450UClamp;
        else
            libCall = Pspv::GLSLstd450SClamp;
        builder.promoteScalar(precision, operands.front(), operands[1]);
        builder.promoteScalar(precision, operands.front(), operands[2]);
        break;
    case Pglslang::EOpMix:
        if (! builder.isBoolType(builder.getScalarTypeId(builder.getTypeId(operands.back())))) {
            assert(isFloat);
            libCall = Pspv::GLSLstd450FMix;
        } else {
            opCode = Pspv::OpSelect;
            std::swap(operands.front(), operands.back());
        }
        builder.promoteScalar(precision, operands.front(), operands.back());
        break;
    case Pglslang::EOpStep:
        libCall = Pspv::GLSLstd450Step;
        builder.promoteScalar(precision, operands.front(), operands.back());
        break;
    case Pglslang::EOpSmoothStep:
        libCall = Pspv::GLSLstd450SmoothStep;
        builder.promoteScalar(precision, operands[0], operands[2]);
        builder.promoteScalar(precision, operands[1], operands[2]);
        break;

    case Pglslang::EOpDistance:
        libCall = Pspv::GLSLstd450Distance;
        break;
    case Pglslang::EOpCross:
        libCall = Pspv::GLSLstd450Cross;
        break;
    case Pglslang::EOpFaceForward:
        libCall = Pspv::GLSLstd450FaceForward;
        break;
    case Pglslang::EOpReflect:
        libCall = Pspv::GLSLstd450Reflect;
        break;
    case Pglslang::EOpRefract:
        libCall = Pspv::GLSLstd450Refract;
        break;
    case Pglslang::EOpBarrier:
        {
            // This is for the extended controlBarrier function, with four operands.
            // The unextended barrier() goes through createNoArgOperation.
            assert(operands.size() == 4);
            unsigned int executionScope = builder.getConstantScalar(operands[0]);
            unsigned int memoryScope = builder.getConstantScalar(operands[1]);
            unsigned int semantics = builder.getConstantScalar(operands[2]) | builder.getConstantScalar(operands[3]);
            builder.createControlBarrier((Pspv::Scope)executionScope, (Pspv::Scope)memoryScope, (Pspv::MemorySemanticsMask)semantics);
            if (semantics & (Pspv::MemorySemanticsMakeAvailableKHRMask |
                             Pspv::MemorySemanticsMakeVisibleKHRMask |
                             Pspv::MemorySemanticsOutputMemoryKHRMask |
                             Pspv::MemorySemanticsVolatileMask)) {
                builder.addCapability(Pspv::CapabilityVulkanMemoryModelKHR);
            }
            if (glslangIntermediate->usingVulkanMemoryModel() && (executionScope == Pspv::ScopeDevice || memoryScope == Pspv::ScopeDevice)) {
                builder.addCapability(Pspv::CapabilityVulkanMemoryModelDeviceScopeKHR);
            }
            return 0;
        }
        break;
    case Pglslang::EOpMemoryBarrier:
        {
            // This is for the extended memoryBarrier function, with three operands.
            // The unextended memoryBarrier() goes through createNoArgOperation.
            assert(operands.size() == 3);
            unsigned int memoryScope = builder.getConstantScalar(operands[0]);
            unsigned int semantics = builder.getConstantScalar(operands[1]) | builder.getConstantScalar(operands[2]);
            builder.createMemoryBarrier((Pspv::Scope)memoryScope, (Pspv::MemorySemanticsMask)semantics);
            if (semantics & (Pspv::MemorySemanticsMakeAvailableKHRMask |
                             Pspv::MemorySemanticsMakeVisibleKHRMask |
                             Pspv::MemorySemanticsOutputMemoryKHRMask |
                             Pspv::MemorySemanticsVolatileMask)) {
                builder.addCapability(Pspv::CapabilityVulkanMemoryModelKHR);
            }
            if (glslangIntermediate->usingVulkanMemoryModel() && memoryScope == Pspv::ScopeDevice) {
                builder.addCapability(Pspv::CapabilityVulkanMemoryModelDeviceScopeKHR);
            }
            return 0;
        }
        break;

#ifndef GLSLANG_WEB
    case Pglslang::EOpInterpolateAtSample:
        if (typeProxy == Pglslang::EbtFloat16)
            builder.addExtension(Pspv::E_SPV_AMD_gpu_shader_half_float);
        libCall = Pspv::GLSLstd450InterpolateAtSample;
        break;
    case Pglslang::EOpInterpolateAtOffset:
        if (typeProxy == Pglslang::EbtFloat16)
            builder.addExtension(Pspv::E_SPV_AMD_gpu_shader_half_float);
        libCall = Pspv::GLSLstd450InterpolateAtOffset;
        break;
    case Pglslang::EOpAddCarry:
        opCode = Pspv::OpIAddCarry;
        typeId = builder.makeStructResultType(typeId0, typeId0);
        consumedOperands = 2;
        break;
    case Pglslang::EOpSubBorrow:
        opCode = Pspv::OpISubBorrow;
        typeId = builder.makeStructResultType(typeId0, typeId0);
        consumedOperands = 2;
        break;
    case Pglslang::EOpUMulExtended:
        opCode = Pspv::OpUMulExtended;
        typeId = builder.makeStructResultType(typeId0, typeId0);
        consumedOperands = 2;
        break;
    case Pglslang::EOpIMulExtended:
        opCode = Pspv::OpSMulExtended;
        typeId = builder.makeStructResultType(typeId0, typeId0);
        consumedOperands = 2;
        break;
    case Pglslang::EOpBitfieldExtract:
        if (isUnsigned)
            opCode = Pspv::OpBitFieldUExtract;
        else
            opCode = Pspv::OpBitFieldSExtract;
        break;
    case Pglslang::EOpBitfieldInsert:
        opCode = Pspv::OpBitFieldInsert;
        break;

    case Pglslang::EOpFma:
        libCall = Pspv::GLSLstd450Fma;
        break;
    case Pglslang::EOpFrexp:
        {
            libCall = Pspv::GLSLstd450FrexpStruct;
            assert(builder.isPointerType(typeId1));
            typeId1 = builder.getContainedTypeId(typeId1);
            int width = builder.getScalarTypeWidth(typeId1);
            if (width == 16)
                // Using 16-bit exp operand, enable extension SPV_AMD_gpu_shader_int16
                builder.addExtension(Pspv::E_SPV_AMD_gpu_shader_int16);
            if (builder.getNumComponents(operands[0]) == 1)
                frexpIntType = builder.makeIntegerType(width, true);
            else
                frexpIntType = builder.makeVectorType(builder.makeIntegerType(width, true), builder.getNumComponents(operands[0]));
            typeId = builder.makeStructResultType(typeId0, frexpIntType);
            consumedOperands = 1;
        }
        break;
    case Pglslang::EOpLdexp:
        libCall = Pspv::GLSLstd450Ldexp;
        break;

    case Pglslang::EOpReadInvocation:
        return createInvocationsOperation(op, typeId, operands, typeProxy);

    case Pglslang::EOpSubgroupBroadcast:
    case Pglslang::EOpSubgroupBallotBitExtract:
    case Pglslang::EOpSubgroupShuffle:
    case Pglslang::EOpSubgroupShuffleXor:
    case Pglslang::EOpSubgroupShuffleUp:
    case Pglslang::EOpSubgroupShuffleDown:
    case Pglslang::EOpSubgroupClusteredAdd:
    case Pglslang::EOpSubgroupClusteredMul:
    case Pglslang::EOpSubgroupClusteredMin:
    case Pglslang::EOpSubgroupClusteredMax:
    case Pglslang::EOpSubgroupClusteredAnd:
    case Pglslang::EOpSubgroupClusteredOr:
    case Pglslang::EOpSubgroupClusteredXor:
    case Pglslang::EOpSubgroupQuadBroadcast:
    case Pglslang::EOpSubgroupPartitionedAdd:
    case Pglslang::EOpSubgroupPartitionedMul:
    case Pglslang::EOpSubgroupPartitionedMin:
    case Pglslang::EOpSubgroupPartitionedMax:
    case Pglslang::EOpSubgroupPartitionedAnd:
    case Pglslang::EOpSubgroupPartitionedOr:
    case Pglslang::EOpSubgroupPartitionedXor:
    case Pglslang::EOpSubgroupPartitionedInclusiveAdd:
    case Pglslang::EOpSubgroupPartitionedInclusiveMul:
    case Pglslang::EOpSubgroupPartitionedInclusiveMin:
    case Pglslang::EOpSubgroupPartitionedInclusiveMax:
    case Pglslang::EOpSubgroupPartitionedInclusiveAnd:
    case Pglslang::EOpSubgroupPartitionedInclusiveOr:
    case Pglslang::EOpSubgroupPartitionedInclusiveXor:
    case Pglslang::EOpSubgroupPartitionedExclusiveAdd:
    case Pglslang::EOpSubgroupPartitionedExclusiveMul:
    case Pglslang::EOpSubgroupPartitionedExclusiveMin:
    case Pglslang::EOpSubgroupPartitionedExclusiveMax:
    case Pglslang::EOpSubgroupPartitionedExclusiveAnd:
    case Pglslang::EOpSubgroupPartitionedExclusiveOr:
    case Pglslang::EOpSubgroupPartitionedExclusiveXor:
        return createSubgroupOperation(op, typeId, operands, typeProxy);

    case Pglslang::EOpSwizzleInvocations:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_shader_ballot);
        libCall = Pspv::SwizzleInvocationsAMD;
        break;
    case Pglslang::EOpSwizzleInvocationsMasked:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_shader_ballot);
        libCall = Pspv::SwizzleInvocationsMaskedAMD;
        break;
    case Pglslang::EOpWriteInvocation:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_shader_ballot);
        libCall = Pspv::WriteInvocationAMD;
        break;

    case Pglslang::EOpMin3:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_shader_trinary_minmax);
        if (isFloat)
            libCall = Pspv::FMin3AMD;
        else {
            if (isUnsigned)
                libCall = Pspv::UMin3AMD;
            else
                libCall = Pspv::SMin3AMD;
        }
        break;
    case Pglslang::EOpMax3:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_shader_trinary_minmax);
        if (isFloat)
            libCall = Pspv::FMax3AMD;
        else {
            if (isUnsigned)
                libCall = Pspv::UMax3AMD;
            else
                libCall = Pspv::SMax3AMD;
        }
        break;
    case Pglslang::EOpMid3:
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_shader_trinary_minmax);
        if (isFloat)
            libCall = Pspv::FMid3AMD;
        else {
            if (isUnsigned)
                libCall = Pspv::UMid3AMD;
            else
                libCall = Pspv::SMid3AMD;
        }
        break;

    case Pglslang::EOpInterpolateAtVertex:
        if (typeProxy == Pglslang::EbtFloat16)
            builder.addExtension(Pspv::E_SPV_AMD_gpu_shader_half_float);
        extBuiltins = getExtBuiltins(Pspv::E_SPV_AMD_shader_explicit_vertex_parameter);
        libCall = Pspv::InterpolateAtVertexAMD;
        break;

    case Pglslang::EOpReportIntersectionNV:
    {
        typeId = builder.makeBoolType();
        opCode = Pspv::OpReportIntersectionNV;
    }
    break;
    case Pglslang::EOpTraceNV:
    {
        builder.createNoResultOp(Pspv::OpTraceNV, operands);
        return 0;
    }
    break;
    case Pglslang::EOpExecuteCallableNV:
    {
        builder.createNoResultOp(Pspv::OpExecuteCallableNV, operands);
        return 0;
    }
    break;
    case Pglslang::EOpWritePackedPrimitiveIndices4x8NV:
        builder.createNoResultOp(Pspv::OpWritePackedPrimitiveIndices4x8NV, operands);
        return 0;
    case Pglslang::EOpCooperativeMatrixMulAdd:
        opCode = Pspv::OpCooperativeMatrixMulAddNV;
        break;
#endif // GLSLANG_WEB
    default:
        return 0;
    }

    Pspv::Id id = 0;
    if (libCall >= 0) {
        // Use an extended instruction from the standard library.
        // Construct the call arguments, without modifying the original operands vector.
        // We might need the remaining arguments, e.g. in the EOpFrexp case.
        std::vector<Pspv::Id> callArguments(operands.begin(), operands.begin() + consumedOperands);
        id = builder.createBuiltinCall(typeId, extBuiltins >= 0 ? extBuiltins : stdBuiltins, libCall, callArguments);
    } else if (opCode == Pspv::OpDot && !isFloat) {
        // int dot(int, int)
        // NOTE: never called for scalar/vector1, this is turned into simple mul before this can be reached
        const int componentCount = builder.getNumComponents(operands[0]);
        Pspv::Id mulOp = builder.createBinOp(Pspv::OpIMul, builder.getTypeId(operands[0]), operands[0], operands[1]);
        builder.setPrecision(mulOp, precision);
        id = builder.createCompositeExtract(mulOp, typeId, 0);
        for (int i = 1; i < componentCount; ++i) {
            builder.setPrecision(id, precision);
            id = builder.createBinOp(Pspv::OpIAdd, typeId, id, builder.createCompositeExtract(mulOp, typeId, i));
        }
    } else {
        switch (consumedOperands) {
        case 0:
            // should all be handled by visitAggregate and createNoArgOperation
            assert(0);
            return 0;
        case 1:
            // should all be handled by createUnaryOperation
            assert(0);
            return 0;
        case 2:
            id = builder.createBinOp(opCode, typeId, operands[0], operands[1]);
            break;
        default:
            // anything 3 or over doesn't have l-value operands, so all should be consumed
            assert(consumedOperands == operands.size());
            id = builder.createOp(opCode, typeId, operands);
            break;
        }
    }

#ifndef GLSLANG_WEB
    // Decode the return types that were structures
    switch (op) {
    case Pglslang::EOpAddCarry:
    case Pglslang::EOpSubBorrow:
        builder.createStore(builder.createCompositeExtract(id, typeId0, 1), operands[2]);
        id = builder.createCompositeExtract(id, typeId0, 0);
        break;
    case Pglslang::EOpUMulExtended:
    case Pglslang::EOpIMulExtended:
        builder.createStore(builder.createCompositeExtract(id, typeId0, 0), operands[3]);
        builder.createStore(builder.createCompositeExtract(id, typeId0, 1), operands[2]);
        break;
    case Pglslang::EOpFrexp:
        {
            assert(operands.size() == 2);
            if (builder.isFloatType(builder.getScalarTypeId(typeId1))) {
                // "exp" is floating-point type (from HLSL intrinsic)
                Pspv::Id member1 = builder.createCompositeExtract(id, frexpIntType, 1);
                member1 = builder.createUnaryOp(Pspv::OpConvertSToF, typeId1, member1);
                builder.createStore(member1, operands[1]);
            } else
                // "exp" is integer type (from GLSL built-in function)
                builder.createStore(builder.createCompositeExtract(id, frexpIntType, 1), operands[1]);
            id = builder.createCompositeExtract(id, typeId0, 0);
        }
        break;
    default:
        break;
    }
#endif

    return builder.setPrecision(id, precision);
}

// Intrinsics with no arguments (or no return value, and no precision).
Pspv::Id TGlslangToSpvTraverser::createNoArgOperation(Pglslang::TOperator op, Pspv::Decoration precision, Pspv::Id typeId)
{
    // GLSL memory barriers use queuefamily scope in new model, device scope in old model
    Pspv::Scope memoryBarrierScope = glslangIntermediate->usingVulkanMemoryModel() ? Pspv::ScopeQueueFamilyKHR : Pspv::ScopeDevice;

    switch (op) {
    case Pglslang::EOpBarrier:
        if (glslangIntermediate->getStage() == EShLangTessControl) {
            if (glslangIntermediate->usingVulkanMemoryModel()) {
                builder.createControlBarrier(Pspv::ScopeWorkgroup, Pspv::ScopeWorkgroup,
                                             Pspv::MemorySemanticsOutputMemoryKHRMask |
                                             Pspv::MemorySemanticsAcquireReleaseMask);
                builder.addCapability(Pspv::CapabilityVulkanMemoryModelKHR);
            } else {
                builder.createControlBarrier(Pspv::ScopeWorkgroup, Pspv::ScopeInvocation, Pspv::MemorySemanticsMaskNone);
            }
        } else {
            builder.createControlBarrier(Pspv::ScopeWorkgroup, Pspv::ScopeWorkgroup,
                                            Pspv::MemorySemanticsWorkgroupMemoryMask |
                                            Pspv::MemorySemanticsAcquireReleaseMask);
        }
        return 0;
    case Pglslang::EOpMemoryBarrier:
        builder.createMemoryBarrier(memoryBarrierScope, Pspv::MemorySemanticsAllMemory |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpMemoryBarrierBuffer:
        builder.createMemoryBarrier(memoryBarrierScope, Pspv::MemorySemanticsUniformMemoryMask |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpMemoryBarrierShared:
        builder.createMemoryBarrier(memoryBarrierScope, Pspv::MemorySemanticsWorkgroupMemoryMask |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpGroupMemoryBarrier:
        builder.createMemoryBarrier(Pspv::ScopeWorkgroup, Pspv::MemorySemanticsAllMemory |
                                                         Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
#ifndef GLSLANG_WEB
    case Pglslang::EOpMemoryBarrierAtomicCounter:
        builder.createMemoryBarrier(memoryBarrierScope, Pspv::MemorySemanticsAtomicCounterMemoryMask |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpMemoryBarrierImage:
        builder.createMemoryBarrier(memoryBarrierScope, Pspv::MemorySemanticsImageMemoryMask |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpAllMemoryBarrierWithGroupSync:
        builder.createControlBarrier(Pspv::ScopeWorkgroup, Pspv::ScopeDevice,
                                        Pspv::MemorySemanticsAllMemory |
                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpDeviceMemoryBarrier:
        builder.createMemoryBarrier(Pspv::ScopeDevice, Pspv::MemorySemanticsUniformMemoryMask |
                                                      Pspv::MemorySemanticsImageMemoryMask |
                                                      Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpDeviceMemoryBarrierWithGroupSync:
        builder.createControlBarrier(Pspv::ScopeWorkgroup, Pspv::ScopeDevice, Pspv::MemorySemanticsUniformMemoryMask |
                                                                            Pspv::MemorySemanticsImageMemoryMask |
                                                                            Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpWorkgroupMemoryBarrier:
        builder.createMemoryBarrier(Pspv::ScopeWorkgroup, Pspv::MemorySemanticsWorkgroupMemoryMask |
                                                         Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpWorkgroupMemoryBarrierWithGroupSync:
        builder.createControlBarrier(Pspv::ScopeWorkgroup, Pspv::ScopeWorkgroup,
                                        Pspv::MemorySemanticsWorkgroupMemoryMask |
                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return 0;
    case Pglslang::EOpSubgroupBarrier:
        builder.createControlBarrier(Pspv::ScopeSubgroup, Pspv::ScopeSubgroup, Pspv::MemorySemanticsAllMemory |
                                                                             Pspv::MemorySemanticsAcquireReleaseMask);
        return Pspv::NoResult;
    case Pglslang::EOpSubgroupMemoryBarrier:
        builder.createMemoryBarrier(Pspv::ScopeSubgroup, Pspv::MemorySemanticsAllMemory |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return Pspv::NoResult;
    case Pglslang::EOpSubgroupMemoryBarrierBuffer:
        builder.createMemoryBarrier(Pspv::ScopeSubgroup, Pspv::MemorySemanticsUniformMemoryMask |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return Pspv::NoResult;
    case Pglslang::EOpSubgroupMemoryBarrierImage:
        builder.createMemoryBarrier(Pspv::ScopeSubgroup, Pspv::MemorySemanticsImageMemoryMask |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return Pspv::NoResult;
    case Pglslang::EOpSubgroupMemoryBarrierShared:
        builder.createMemoryBarrier(Pspv::ScopeSubgroup, Pspv::MemorySemanticsWorkgroupMemoryMask |
                                                        Pspv::MemorySemanticsAcquireReleaseMask);
        return Pspv::NoResult;

    case Pglslang::EOpEmitVertex:
        builder.createNoResultOp(Pspv::OpEmitVertex);
        return 0;
    case Pglslang::EOpEndPrimitive:
        builder.createNoResultOp(Pspv::OpEndPrimitive);
        return 0;

    case Pglslang::EOpSubgroupElect: {
        std::vector<Pspv::Id> operands;
        return createSubgroupOperation(op, typeId, operands, Pglslang::EbtVoid);
    }
    case Pglslang::EOpTime:
    {
        std::vector<Pspv::Id> args; // Dummy arguments
        Pspv::Id id = builder.createBuiltinCall(typeId, getExtBuiltins(Pspv::E_SPV_AMD_gcn_shader), Pspv::TimeAMD, args);
        return builder.setPrecision(id, precision);
    }
    case Pglslang::EOpIgnoreIntersectionNV:
        builder.createNoResultOp(Pspv::OpIgnoreIntersectionNV);
        return 0;
    case Pglslang::EOpTerminateRayNV:
        builder.createNoResultOp(Pspv::OpTerminateRayNV);
        return 0;

    case Pglslang::EOpBeginInvocationInterlock:
        builder.createNoResultOp(Pspv::OpBeginInvocationInterlockEXT);
        return 0;
    case Pglslang::EOpEndInvocationInterlock:
        builder.createNoResultOp(Pspv::OpEndInvocationInterlockEXT);
        return 0;

    case Pglslang::EOpIsHelperInvocation:
    {
        std::vector<Pspv::Id> args; // Dummy arguments
        builder.addExtension(Pspv::E_SPV_EXT_demote_to_helper_invocation);
        builder.addCapability(Pspv::CapabilityDemoteToHelperInvocationEXT);
        return builder.createOp(Pspv::OpIsHelperInvocationEXT, typeId, args);
    }

    case Pglslang::EOpReadClockSubgroupKHR: {
        std::vector<Pspv::Id> args;
        args.push_back(builder.makeUintConstant(Pspv::ScopeSubgroup));
        builder.addExtension(Pspv::E_SPV_KHR_shader_clock);
        builder.addCapability(Pspv::CapabilityShaderClockKHR);
        return builder.createOp(Pspv::OpReadClockKHR, typeId, args);
    }

    case Pglslang::EOpReadClockDeviceKHR: {
        std::vector<Pspv::Id> args;
        args.push_back(builder.makeUintConstant(Pspv::ScopeDevice));
        builder.addExtension(Pspv::E_SPV_KHR_shader_clock);
        builder.addCapability(Pspv::CapabilityShaderClockKHR);
        return builder.createOp(Pspv::OpReadClockKHR, typeId, args);
    }
#endif
    default:
        break;
    }

    logger->missingFunctionality("unknown operation with no arguments");

    return 0;
}

Pspv::Id TGlslangToSpvTraverser::getSymbolId(const Pglslang::TIntermSymbol* symbol)
{
    auto iter = symbolValues.find(symbol->getId());
    Pspv::Id id;
    if (symbolValues.end() != iter) {
        id = iter->second;
        return id;
    }

    // it was not found, create it
    Pspv::BuiltIn builtIn = TranslateBuiltInDecoration(symbol->getQualifier().builtIn, false);
    auto forcedType = getForcedType(builtIn, symbol->getType());
    id = createSpvVariable(symbol, forcedType.first);
    symbolValues[symbol->getId()] = id;
    if (forcedType.second != Pspv::NoType)
        forceType[id] = forcedType.second;

    if (symbol->getBasicType() != Pglslang::EbtBlock) {
        builder.addDecoration(id, TranslatePrecisionDecoration(symbol->getType()));
        builder.addDecoration(id, TranslateInterpolationDecoration(symbol->getType().getQualifier()));
        builder.addDecoration(id, TranslateAuxiliaryStorageDecoration(symbol->getType().getQualifier()));
#ifndef GLSLANG_WEB
        addMeshNVDecoration(id, /*member*/ -1, symbol->getType().getQualifier());
        if (symbol->getQualifier().hasComponent())
            builder.addDecoration(id, Pspv::DecorationComponent, symbol->getQualifier().layoutComponent);
        if (symbol->getQualifier().hasIndex())
            builder.addDecoration(id, Pspv::DecorationIndex, symbol->getQualifier().layoutIndex);
#endif
        if (symbol->getType().getQualifier().hasSpecConstantId())
            builder.addDecoration(id, Pspv::DecorationSpecId, symbol->getType().getQualifier().layoutSpecConstantId);
        // atomic counters use this:
        if (symbol->getQualifier().hasOffset())
            builder.addDecoration(id, Pspv::DecorationOffset, symbol->getQualifier().layoutOffset);
    }

    if (symbol->getQualifier().hasLocation())
        builder.addDecoration(id, Pspv::DecorationLocation, symbol->getQualifier().layoutLocation);
    builder.addDecoration(id, TranslateInvariantDecoration(symbol->getType().getQualifier()));
    if (symbol->getQualifier().hasStream() && glslangIntermediate->isMultiStream()) {
        builder.addCapability(Pspv::CapabilityGeometryStreams);
        builder.addDecoration(id, Pspv::DecorationStream, symbol->getQualifier().layoutStream);
    }
    if (symbol->getQualifier().hasSet())
        builder.addDecoration(id, Pspv::DecorationDescriptorSet, symbol->getQualifier().layoutSet);
    else if (IsDescriptorResource(symbol->getType())) {
        // default to 0
        builder.addDecoration(id, Pspv::DecorationDescriptorSet, 0);
    }
    if (symbol->getQualifier().hasBinding())
        builder.addDecoration(id, Pspv::DecorationBinding, symbol->getQualifier().layoutBinding);
    else if (IsDescriptorResource(symbol->getType())) {
        // default to 0
        builder.addDecoration(id, Pspv::DecorationBinding, 0);
    }
    if (symbol->getQualifier().hasAttachment())
        builder.addDecoration(id, Pspv::DecorationInputAttachmentIndex, symbol->getQualifier().layoutAttachment);
    if (glslangIntermediate->getXfbMode()) {
        builder.addCapability(Pspv::CapabilityTransformFeedback);
        if (symbol->getQualifier().hasXfbBuffer()) {
            builder.addDecoration(id, Pspv::DecorationXfbBuffer, symbol->getQualifier().layoutXfbBuffer);
            unsigned stride = glslangIntermediate->getXfbStride(symbol->getQualifier().layoutXfbBuffer);
            if (stride != Pglslang::TQualifier::layoutXfbStrideEnd)
                builder.addDecoration(id, Pspv::DecorationXfbStride, stride);
        }
        if (symbol->getQualifier().hasXfbOffset())
            builder.addDecoration(id, Pspv::DecorationOffset, symbol->getQualifier().layoutXfbOffset);
    }

    // add built-in variable decoration
    if (builtIn != Pspv::BuiltInMax) {
        builder.addDecoration(id, Pspv::DecorationBuiltIn, (int)builtIn);
    }

#ifndef GLSLANG_WEB
    if (symbol->getType().isImage()) {
        std::vector<Pspv::Decoration> memory;
        TranslateMemoryDecoration(symbol->getType().getQualifier(), memory, glslangIntermediate->usingVulkanMemoryModel());
        for (unsigned int i = 0; i < memory.size(); ++i)
            builder.addDecoration(id, memory[i]);
    }

    // nonuniform
    builder.addDecoration(id, TranslateNonUniformDecoration(symbol->getType().getQualifier()));

    if (builtIn == Pspv::BuiltInSampleMask) {
          Pspv::Decoration decoration;
          // GL_NV_sample_mask_override_coverage extension
          if (glslangIntermediate->getLayoutOverrideCoverage())
              decoration = (Pspv::Decoration)Pspv::DecorationOverrideCoverageNV;
          else
              decoration = (Pspv::Decoration)Pspv::DecorationMax;
        builder.addDecoration(id, decoration);
        if (decoration != Pspv::DecorationMax) {
            builder.addCapability(Pspv::CapabilitySampleMaskOverrideCoverageNV);
            builder.addExtension(Pspv::E_SPV_NV_sample_mask_override_coverage);
        }
    }
    else if (builtIn == Pspv::BuiltInLayer) {
        // SPV_NV_viewport_array2 extension
        if (symbol->getQualifier().layoutViewportRelative) {
            builder.addDecoration(id, (Pspv::Decoration)Pspv::DecorationViewportRelativeNV);
            builder.addCapability(Pspv::CapabilityShaderViewportMaskNV);
            builder.addExtension(Pspv::E_SPV_NV_viewport_array2);
        }
        if (symbol->getQualifier().layoutSecondaryViewportRelativeOffset != -2048) {
            builder.addDecoration(id, (Pspv::Decoration)Pspv::DecorationSecondaryViewportRelativeNV,
                                  symbol->getQualifier().layoutSecondaryViewportRelativeOffset);
            builder.addCapability(Pspv::CapabilityShaderStereoViewNV);
            builder.addExtension(Pspv::E_SPV_NV_stereo_view_rendering);
        }
    }

    if (symbol->getQualifier().layoutPassthrough) {
        builder.addDecoration(id, Pspv::DecorationPassthroughNV);
        builder.addCapability(Pspv::CapabilityGeometryShaderPassthroughNV);
        builder.addExtension(Pspv::E_SPV_NV_geometry_shader_passthrough);
    }
    if (symbol->getQualifier().pervertexNV) {
        builder.addDecoration(id, Pspv::DecorationPerVertexNV);
        builder.addCapability(Pspv::CapabilityFragmentBarycentricNV);
        builder.addExtension(Pspv::E_SPV_NV_fragment_shader_barycentric);
    }

    if (glslangIntermediate->getHlslFunctionality1() && symbol->getType().getQualifier().semanticName != nullptr) {
        builder.addExtension("SPV_GOOGLE_hlsl_functionality1");
        builder.addDecoration(id, (Pspv::Decoration)Pspv::DecorationHlslSemanticGOOGLE,
                              symbol->getType().getQualifier().semanticName);
    }

    if (symbol->isReference()) {
        builder.addDecoration(id, symbol->getType().getQualifier().restrict ? Pspv::DecorationRestrictPointerEXT : Pspv::DecorationAliasedPointerEXT);
    }
#endif

    return id;
}

#ifndef GLSLANG_WEB
// add per-primitive, per-view. per-task decorations to a struct member (member >= 0) or an object
void TGlslangToSpvTraverser::addMeshNVDecoration(Pspv::Id id, int member, const Pglslang::TQualifier& qualifier)
{
    if (member >= 0) {
        if (qualifier.perPrimitiveNV) {
            // Need to add capability/extension for fragment shader.
            // Mesh shader already adds this by default.
            if (glslangIntermediate->getStage() == EShLangFragment) {
                builder.addCapability(Pspv::CapabilityMeshShadingNV);
                builder.addExtension(Pspv::E_SPV_NV_mesh_shader);
            }
            builder.addMemberDecoration(id, (unsigned)member, Pspv::DecorationPerPrimitiveNV);
        }
        if (qualifier.perViewNV)
            builder.addMemberDecoration(id, (unsigned)member, Pspv::DecorationPerViewNV);
        if (qualifier.perTaskNV)
            builder.addMemberDecoration(id, (unsigned)member, Pspv::DecorationPerTaskNV);
    } else {
        if (qualifier.perPrimitiveNV) {
            // Need to add capability/extension for fragment shader.
            // Mesh shader already adds this by default.
            if (glslangIntermediate->getStage() == EShLangFragment) {
                builder.addCapability(Pspv::CapabilityMeshShadingNV);
                builder.addExtension(Pspv::E_SPV_NV_mesh_shader);
            }
            builder.addDecoration(id, Pspv::DecorationPerPrimitiveNV);
        }
        if (qualifier.perViewNV)
            builder.addDecoration(id, Pspv::DecorationPerViewNV);
        if (qualifier.perTaskNV)
            builder.addDecoration(id, Pspv::DecorationPerTaskNV);
    }
}
#endif

// Make a full tree of instructions to build a SPIR-V specialization constant,
// or regular constant if possible.
//
// TBD: this is not yet done, nor verified to be the best design, it does do the leaf symbols though
//
// Recursively walk the nodes.  The nodes form a tree whose leaves are
// regular constants, which themselves are trees that createSpvConstant()
// recursively walks.  So, this function walks the "top" of the tree:
//  - emit specialization constant-building instructions for specConstant
//  - when running into a non-spec-constant, switch to createSpvConstant()
Pspv::Id TGlslangToSpvTraverser::createSpvConstant(const Pglslang::TIntermTyped& node)
{
    assert(node.getQualifier().isConstant());

    // Handle front-end constants first (non-specialization constants).
    if (! node.getQualifier().specConstant) {
        // hand off to the non-spec-constant path
        assert(node.getAsConstantUnion() != nullptr || node.getAsSymbolNode() != nullptr);
        int nextConst = 0;
        return createSpvConstantFromConstUnionArray(node.getType(), node.getAsConstantUnion() ? node.getAsConstantUnion()->getConstArray() : node.getAsSymbolNode()->getConstArray(),
                                 nextConst, false);
    }

    // We now know we have a specialization constant to build

    // gl_WorkGroupSize is a special case until the front-end handles hierarchical specialization constants,
    // even then, it's specialization ids are handled by special case syntax in GLSL: layout(local_size_x = ...
    if (node.getType().getQualifier().builtIn == Pglslang::EbvWorkGroupSize) {
        std::vector<Pspv::Id> dimConstId;
        for (int dim = 0; dim < 3; ++dim) {
            bool specConst = (glslangIntermediate->getLocalSizeSpecId(dim) != Pglslang::TQualifier::layoutNotSet);
            dimConstId.push_back(builder.makeUintConstant(glslangIntermediate->getLocalSize(dim), specConst));
            if (specConst) {
                builder.addDecoration(dimConstId.back(), Pspv::DecorationSpecId,
                                      glslangIntermediate->getLocalSizeSpecId(dim));
            }
        }
        return builder.makeCompositeConstant(builder.makeVectorType(builder.makeUintType(32), 3), dimConstId, true);
    }

    // An AST node labelled as specialization constant should be a symbol node.
    // Its initializer should either be a sub tree with constant nodes, or a constant union array.
    if (auto* sn = node.getAsSymbolNode()) {
        Pspv::Id result;
        if (auto* sub_tree = sn->getConstSubtree()) {
            // Traverse the constant constructor sub tree like generating normal run-time instructions.
            // During the AST traversal, if the node is marked as 'specConstant', SpecConstantOpModeGuard
            // will set the builder into spec constant op instruction generating mode.
            sub_tree->traverse(this);
            result = accessChainLoad(sub_tree->getType());
        } else if (auto* const_union_array = &sn->getConstArray()) {
            int nextConst = 0;
            result = createSpvConstantFromConstUnionArray(sn->getType(), *const_union_array, nextConst, true);
        } else {
            logger->missingFunctionality("Invalid initializer for spec onstant.");
            return Pspv::NoResult;
        }
        builder.addName(result, sn->getName().c_str());
        return result;
    }

    // Neither a front-end constant node, nor a specialization constant node with constant union array or
    // constant sub tree as initializer.
    logger->missingFunctionality("Neither a front-end constant nor a spec constant.");
    return Pspv::NoResult;
}

// Use 'consts' as the flattened glslang source of scalar constants to recursively
// build the aggregate SPIR-V constant.
//
// If there are not enough elements present in 'consts', 0 will be substituted;
// an empty 'consts' can be used to create a fully zeroed SPIR-V constant.
//
Pspv::Id TGlslangToSpvTraverser::createSpvConstantFromConstUnionArray(const Pglslang::TType& glslangType, const Pglslang::TConstUnionArray& consts, int& nextConst, bool specConstant)
{
    // vector of constants for SPIR-V
    std::vector<Pspv::Id> spvConsts;

    // Type is used for struct and array constants
    Pspv::Id typeId = convertGlslangToSpvType(glslangType);

    if (glslangType.isArray()) {
        Pglslang::TType elementType(glslangType, 0);
        for (int i = 0; i < glslangType.getOuterArraySize(); ++i)
            spvConsts.push_back(createSpvConstantFromConstUnionArray(elementType, consts, nextConst, false));
    } else if (glslangType.isMatrix()) {
        Pglslang::TType vectorType(glslangType, 0);
        for (int col = 0; col < glslangType.getMatrixCols(); ++col)
            spvConsts.push_back(createSpvConstantFromConstUnionArray(vectorType, consts, nextConst, false));
    } else if (glslangType.isCoopMat()) {
        Pglslang::TType componentType(glslangType.getBasicType());
        spvConsts.push_back(createSpvConstantFromConstUnionArray(componentType, consts, nextConst, false));
    } else if (glslangType.isStruct()) {
        Pglslang::TVector<Pglslang::TTypeLoc>::const_iterator iter;
        for (iter = glslangType.getStruct()->begin(); iter != glslangType.getStruct()->end(); ++iter)
            spvConsts.push_back(createSpvConstantFromConstUnionArray(*iter->type, consts, nextConst, false));
    } else if (glslangType.getVectorSize() > 1) {
        for (unsigned int i = 0; i < (unsigned int)glslangType.getVectorSize(); ++i) {
            bool zero = nextConst >= consts.size();
            switch (glslangType.getBasicType()) {
            case Pglslang::EbtInt:
                spvConsts.push_back(builder.makeIntConstant(zero ? 0 : consts[nextConst].getIConst()));
                break;
            case Pglslang::EbtUint:
                spvConsts.push_back(builder.makeUintConstant(zero ? 0 : consts[nextConst].getUConst()));
                break;
            case Pglslang::EbtFloat:
                spvConsts.push_back(builder.makeFloatConstant(zero ? 0.0F : (float)consts[nextConst].getDConst()));
                break;
            case Pglslang::EbtBool:
                spvConsts.push_back(builder.makeBoolConstant(zero ? false : consts[nextConst].getBConst()));
                break;
#ifndef GLSLANG_WEB
            case Pglslang::EbtInt8:
                spvConsts.push_back(builder.makeInt8Constant(zero ? 0 : consts[nextConst].getI8Const()));
                break;
            case Pglslang::EbtUint8:
                spvConsts.push_back(builder.makeUint8Constant(zero ? 0 : consts[nextConst].getU8Const()));
                break;
            case Pglslang::EbtInt16:
                spvConsts.push_back(builder.makeInt16Constant(zero ? 0 : consts[nextConst].getI16Const()));
                break;
            case Pglslang::EbtUint16:
                spvConsts.push_back(builder.makeUint16Constant(zero ? 0 : consts[nextConst].getU16Const()));
                break;
            case Pglslang::EbtInt64:
                spvConsts.push_back(builder.makeInt64Constant(zero ? 0 : consts[nextConst].getI64Const()));
                break;
            case Pglslang::EbtUint64:
                spvConsts.push_back(builder.makeUint64Constant(zero ? 0 : consts[nextConst].getU64Const()));
                break;
            case Pglslang::EbtDouble:
                spvConsts.push_back(builder.makeDoubleConstant(zero ? 0.0 : consts[nextConst].getDConst()));
                break;
            case Pglslang::EbtFloat16:
                spvConsts.push_back(builder.makeFloat16Constant(zero ? 0.0F : (float)consts[nextConst].getDConst()));
                break;
#endif
            default:
                assert(0);
                break;
            }
            ++nextConst;
        }
    } else {
        // we have a non-aggregate (scalar) constant
        bool zero = nextConst >= consts.size();
        Pspv::Id scalar = 0;
        switch (glslangType.getBasicType()) {
        case Pglslang::EbtInt:
            scalar = builder.makeIntConstant(zero ? 0 : consts[nextConst].getIConst(), specConstant);
            break;
        case Pglslang::EbtUint:
            scalar = builder.makeUintConstant(zero ? 0 : consts[nextConst].getUConst(), specConstant);
            break;
        case Pglslang::EbtFloat:
            scalar = builder.makeFloatConstant(zero ? 0.0F : (float)consts[nextConst].getDConst(), specConstant);
            break;
        case Pglslang::EbtBool:
            scalar = builder.makeBoolConstant(zero ? false : consts[nextConst].getBConst(), specConstant);
            break;
#ifndef GLSLANG_WEB
        case Pglslang::EbtInt8:
            scalar = builder.makeInt8Constant(zero ? 0 : consts[nextConst].getI8Const(), specConstant);
            break;
        case Pglslang::EbtUint8:
            scalar = builder.makeUint8Constant(zero ? 0 : consts[nextConst].getU8Const(), specConstant);
            break;
        case Pglslang::EbtInt16:
            scalar = builder.makeInt16Constant(zero ? 0 : consts[nextConst].getI16Const(), specConstant);
            break;
        case Pglslang::EbtUint16:
            scalar = builder.makeUint16Constant(zero ? 0 : consts[nextConst].getU16Const(), specConstant);
            break;
        case Pglslang::EbtInt64:
            scalar = builder.makeInt64Constant(zero ? 0 : consts[nextConst].getI64Const(), specConstant);
            break;
        case Pglslang::EbtUint64:
            scalar = builder.makeUint64Constant(zero ? 0 : consts[nextConst].getU64Const(), specConstant);
            break;
        case Pglslang::EbtDouble:
            scalar = builder.makeDoubleConstant(zero ? 0.0 : consts[nextConst].getDConst(), specConstant);
            break;
        case Pglslang::EbtFloat16:
            scalar = builder.makeFloat16Constant(zero ? 0.0F : (float)consts[nextConst].getDConst(), specConstant);
            break;
        case Pglslang::EbtReference:
            scalar = builder.makeUint64Constant(zero ? 0 : consts[nextConst].getU64Const(), specConstant);
            scalar = builder.createUnaryOp(Pspv::OpBitcast, typeId, scalar);
            break;
#endif
        default:
            assert(0);
            break;
        }
        ++nextConst;
        return scalar;
    }

    return builder.makeCompositeConstant(typeId, spvConsts);
}

// Return true if the node is a constant or symbol whose reading has no
// non-trivial observable cost or effect.
bool TGlslangToSpvTraverser::isTrivialLeaf(const Pglslang::TIntermTyped* node)
{
    // don't know what this is
    if (node == nullptr)
        return false;

    // a constant is safe
    if (node->getAsConstantUnion() != nullptr)
        return true;

    // not a symbol means non-trivial
    if (node->getAsSymbolNode() == nullptr)
        return false;

    // a symbol, depends on what's being read
    switch (node->getType().getQualifier().storage) {
    case Pglslang::EvqTemporary:
    case Pglslang::EvqGlobal:
    case Pglslang::EvqIn:
    case Pglslang::EvqInOut:
    case Pglslang::EvqConst:
    case Pglslang::EvqConstReadOnly:
    case Pglslang::EvqUniform:
        return true;
    default:
        return false;
    }
}

// A node is trivial if it is a single operation with no side effects.
// HLSL (and/or vectors) are always trivial, as it does not short circuit.
// Otherwise, error on the side of saying non-trivial.
// Return true if trivial.
bool TGlslangToSpvTraverser::isTrivial(const Pglslang::TIntermTyped* node)
{
    if (node == nullptr)
        return false;

    // count non scalars as trivial, as well as anything coming from HLSL
    if (! node->getType().isScalarOrVec1() || glslangIntermediate->getSource() == Pglslang::EShSourceHlsl)
        return true;

    // symbols and constants are trivial
    if (isTrivialLeaf(node))
        return true;

    // otherwise, it needs to be a simple operation or one or two leaf nodes

    // not a simple operation
    const Pglslang::TIntermBinary* binaryNode = node->getAsBinaryNode();
    const Pglslang::TIntermUnary* unaryNode = node->getAsUnaryNode();
    if (binaryNode == nullptr && unaryNode == nullptr)
        return false;

    // not on leaf nodes
    if (binaryNode && (! isTrivialLeaf(binaryNode->getLeft()) || ! isTrivialLeaf(binaryNode->getRight())))
        return false;

    if (unaryNode && ! isTrivialLeaf(unaryNode->getOperand())) {
        return false;
    }

    switch (node->getAsOperator()->getOp()) {
    case Pglslang::EOpLogicalNot:
    case Pglslang::EOpConvIntToBool:
    case Pglslang::EOpConvUintToBool:
    case Pglslang::EOpConvFloatToBool:
    case Pglslang::EOpConvDoubleToBool:
    case Pglslang::EOpEqual:
    case Pglslang::EOpNotEqual:
    case Pglslang::EOpLessThan:
    case Pglslang::EOpGreaterThan:
    case Pglslang::EOpLessThanEqual:
    case Pglslang::EOpGreaterThanEqual:
    case Pglslang::EOpIndexDirect:
    case Pglslang::EOpIndexDirectStruct:
    case Pglslang::EOpLogicalXor:
    case Pglslang::EOpAny:
    case Pglslang::EOpAll:
        return true;
    default:
        return false;
    }
}

// Emit short-circuiting code, where 'right' is never evaluated unless
// the left side is true (for &&) or false (for ||).
Pspv::Id TGlslangToSpvTraverser::createShortCircuit(Pglslang::TOperator op, Pglslang::TIntermTyped& left, Pglslang::TIntermTyped& right)
{
    Pspv::Id boolTypeId = builder.makeBoolType();

    // emit left operand
    builder.clearAccessChain();
    left.traverse(this);
    Pspv::Id leftId = accessChainLoad(left.getType());

    // Operands to accumulate OpPhi operands
    std::vector<Pspv::Id> phiOperands;
    // accumulate left operand's phi information
    phiOperands.push_back(leftId);
    phiOperands.push_back(builder.getBuildPoint()->getId());

    // Make the two kinds of operation symmetric with a "!"
    //   || => emit "if (! left) result = right"
    //   && => emit "if (  left) result = right"
    //
    // TODO: this runtime "not" for || could be avoided by adding functionality
    // to 'builder' to have an "else" without an "then"
    if (op == Pglslang::EOpLogicalOr)
        leftId = builder.createUnaryOp(Pspv::OpLogicalNot, boolTypeId, leftId);

    // make an "if" based on the left value
    Pspv::Builder::If ifBuilder(leftId, Pspv::SelectionControlMaskNone, builder);

    // emit right operand as the "then" part of the "if"
    builder.clearAccessChain();
    right.traverse(this);
    Pspv::Id rightId = accessChainLoad(right.getType());

    // accumulate left operand's phi information
    phiOperands.push_back(rightId);
    phiOperands.push_back(builder.getBuildPoint()->getId());

    // finish the "if"
    ifBuilder.makeEndIf();

    // phi together the two results
    return builder.createOp(Pspv::OpPhi, boolTypeId, phiOperands);
}

#ifndef GLSLANG_WEB
// Return type Id of the imported set of extended instructions corresponds to the name.
// Import this set if it has not been imported yet.
Pspv::Id TGlslangToSpvTraverser::getExtBuiltins(const char* name)
{
    if (extBuiltinMap.find(name) != extBuiltinMap.end())
        return extBuiltinMap[name];
    else {
        builder.addExtension(name);
        Pspv::Id extBuiltins = builder.import(name);
        extBuiltinMap[name] = extBuiltins;
        return extBuiltins;
    }
}
#endif

};  // end anonymous namespace

namespace Pglslang {

void GetSpirvVersion(std::string& version)
{
    const int bufSize = 100;
    char buf[bufSize];
    snprintf(buf, bufSize, "0x%08x, Revision %d", Pspv::Version, Pspv::Revision);
    version = buf;
}

// For low-order part of the generator's magic number. Bump up
// when there is a change in the style (e.g., if SSA form changes,
// or a different instruction sequence to do something gets used).
int GetSpirvGeneratorVersion()
{
    // return 1; // start
    // return 2; // EOpAtomicCounterDecrement gets a post decrement, to map between GLSL -> SPIR-V
    // return 3; // change/correct barrier-instruction operands, to match memory model group decisions
    // return 4; // some deeper access chains: for dynamic vector component, and local Boolean component
    // return 5; // make OpArrayLength result type be an int with signedness of 0
    // return 6; // revert version 5 change, which makes a different (new) kind of incorrect code,
                 // versions 4 and 6 each generate OpArrayLength as it has long been done
    // return 7; // GLSL volatile keyword maps to both SPIR-V decorations Volatile and Coherent
    return 8; // switch to new dead block eliminator; use OpUnreachable
}

// Write SPIR-V out to a binary file
void OutputSpvBin(const std::vector<unsigned int>& spirv, const char* baseName)
{
    std::ofstream out;
    out.open(baseName, std::ios::binary | std::ios::out);
    if (out.fail())
        printf("ERROR: Failed to open file: %s\n", baseName);
    for (int i = 0; i < (int)spirv.size(); ++i) {
        unsigned int word = spirv[i];
        out.write((const char*)&word, 4);
    }
    out.close();
}

// Write SPIR-V out to a text file with 32-bit hexadecimal words
void OutputSpvHex(const std::vector<unsigned int>& spirv, const char* baseName, const char* varName)
{
#ifndef GLSLANG_WEB
    std::ofstream out;
    out.open(baseName, std::ios::binary | std::ios::out);
    if (out.fail())
        printf("ERROR: Failed to open file: %s\n", baseName);
    out << "\t// " << 
        GetSpirvGeneratorVersion() << "." << GLSLANG_MINOR_VERSION << "." << GLSLANG_PATCH_LEVEL <<
        std::endl;
    if (varName != nullptr) {
        out << "\t #pragma once" << std::endl;
        out << "const uint32_t " << varName << "[] = {" << std::endl;
    }
    const int WORDS_PER_LINE = 8;
    for (int i = 0; i < (int)spirv.size(); i += WORDS_PER_LINE) {
        out << "\t";
        for (int j = 0; j < WORDS_PER_LINE && i + j < (int)spirv.size(); ++j) {
            const unsigned int word = spirv[i + j];
            out << "0x" << std::hex << std::setw(8) << std::setfill('0') << word;
            if (i + j + 1 < (int)spirv.size()) {
                out << ",";
            }
        }
        out << std::endl;
    }
    if (varName != nullptr) {
        out << "};";
    }
    out.close();
#endif
}

//
// Set up the glslang traversal
//
void GlslangToSpv(const TIntermediate& intermediate, std::vector<unsigned int>& spirv, SpvOptions* options)
{
    Pspv::SpvBuildLogger logger;
    GlslangToSpv(intermediate, spirv, &logger, options);
}

void GlslangToSpv(const TIntermediate& intermediate, std::vector<unsigned int>& spirv,
                  Pspv::SpvBuildLogger* logger, SpvOptions* options)
{
    TIntermNode* root = intermediate.getTreeRoot();

    if (root == 0)
        return;

    SpvOptions defaultOptions;
    if (options == nullptr)
        options = &defaultOptions;

    GetThreadPoolAllocator().push();

    TGlslangToSpvTraverser it(intermediate.getSpv().spv, &intermediate, logger, *options);
    root->traverse(&it);
    it.finishSpv();
    it.dumpSpv(spirv);

#if ENABLE_OPT
    // If from HLSL, run spirv-opt to "legalize" the SPIR-V for Vulkan
    // eg. forward and remove memory writes of opaque types.
    bool prelegalization = intermediate.getSource() == EShSourceHlsl;
    if ((intermediate.getSource() == EShSourceHlsl || options->optimizeSize) && !options->disableOptimizer) {
        SpirvToolsLegalize(intermediate, spirv, logger, options);
        prelegalization = false;
    }

    if (options->validate)
        SpirvToolsValidate(intermediate, spirv, logger, prelegalization);

    if (options->disassemble)
        SpirvToolsDisassemble(std::cout, spirv);

#endif

    GetThreadPoolAllocator().pop();
}

}; // end namespace Pglslang
