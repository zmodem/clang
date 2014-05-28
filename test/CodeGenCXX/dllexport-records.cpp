// RUN: %clang_cc1 -triple i686-windows-msvc   -emit-llvm -std=c++1y -O0 -o - %s -DMSABI | FileCheck --check-prefix=MSC --check-prefix=M32 %s
// RUN: %clang_cc1 -triple x86_64-windows-msvc -emit-llvm -std=c++1y -O0 -o - %s -DMSABI | FileCheck --check-prefix=MSC --check-prefix=M64 %s
// RUN: %clang_cc1 -triple i686-windows-gnu    -emit-llvm -std=c++1y -O0 -o - %s         | FileCheck --check-prefix=GNU --check-prefix=G32 %s
// RUN: %clang_cc1 -triple x86_64-windows-gnu  -emit-llvm -std=c++1y -O0 -o - %s         | FileCheck --check-prefix=GNU --check-prefix=G64 %s

// Helper structs to make template specializations more expressive.
struct ImplicitInst_Exported {};
struct ExplicitDecl_Exported {};
struct ExplicitInst_Exported {};
struct ExplicitSpec_Exported {};
struct ExplicitSpec_NotExported {};

extern "C" void* malloc(__SIZE_TYPE__ size);
extern "C" void free(void* p);

// Used to force non-trivial special members.
struct ForceNonTrivial {
  ForceNonTrivial();
  ~ForceNonTrivial();
  ForceNonTrivial(const ForceNonTrivial&);
  ForceNonTrivial& operator=(const ForceNonTrivial&);
  ForceNonTrivial(ForceNonTrivial&&);
  ForceNonTrivial& operator=(ForceNonTrivial&&);
};



//===----------------------------------------------------------------------===//
// Classes
//===----------------------------------------------------------------------===//

// Export non-dynamic class.
struct __declspec(dllexport) ExportClass {
  struct Nested {
    // M32-DAG: define x86_thiscallcc void @"\01?normalDef@Nested@ExportClass@@QAEXXZ"(%"struct.ExportClass::Nested"* %this)
    // M64-DAG: define                void @"\01?normalDef@Nested@ExportClass@@QEAAXXZ"(%"struct.ExportClass::Nested"* %this)
    // G32-DAG: define x86_thiscallcc void @_ZN11ExportClass6Nested9normalDefEv(%"struct.ExportClass::Nested"* %this)
    // G64-DAG: define                void @_ZN11ExportClass6Nested9normalDefEv(%"struct.ExportClass::Nested"* %this)
    void normalDef();
  };

  struct __declspec(dllexport) ExportNested {
    // M32-DAG: define dllexport x86_thiscallcc void @"\01?normalDef@ExportNested@ExportClass@@QAEXXZ"(%"struct.ExportClass::ExportNested"* %this)
    // M64-DAG: define dllexport                void @"\01?normalDef@ExportNested@ExportClass@@QEAAXXZ"(%"struct.ExportClass::ExportNested"* %this)
    // G32-DAG: define dllexport x86_thiscallcc void @_ZN11ExportClass12ExportNested9normalDefEv(%"struct.ExportClass::ExportNested"* %this)
    // G64-DAG: define dllexport                void @_ZN11ExportClass12ExportNested9normalDefEv(%"struct.ExportClass::ExportNested"* %this)
    void normalDef();
  };

  // M32-DAG: define          dllexport x86_thiscallcc void @"\01?normalDef@ExportClass@@QAEXXZ"(%struct.ExportClass* %this)
  // M64-DAG: define          dllexport                void @"\01?normalDef@ExportClass@@QEAAXXZ"(%struct.ExportClass* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInclass@ExportClass@@QAEXXZ"(%struct.ExportClass* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?normalInclass@ExportClass@@QEAAXXZ"(%struct.ExportClass* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDef@ExportClass@@QAEXXZ"(%struct.ExportClass* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDef@ExportClass@@QEAAXXZ"(%struct.ExportClass* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDecl@ExportClass@@QAEXXZ"(%struct.ExportClass* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDecl@ExportClass@@QEAAXXZ"(%struct.ExportClass* %this)
  // G32-DAG: define          dllexport x86_thiscallcc void @_ZN11ExportClass9normalDefEv(%struct.ExportClass* %this)
  // G64-DAG: define          dllexport                void @_ZN11ExportClass9normalDefEv(%struct.ExportClass* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN11ExportClass13normalInclassEv(%struct.ExportClass* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN11ExportClass13normalInclassEv(%struct.ExportClass* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN11ExportClass15normalInlineDefEv(%struct.ExportClass* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN11ExportClass15normalInlineDefEv(%struct.ExportClass* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN11ExportClass16normalInlineDeclEv(%struct.ExportClass* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN11ExportClass16normalInlineDeclEv(%struct.ExportClass* %this)
                void normalDef();
                void normalInclass() {}
                void normalInlineDef();
         inline void normalInlineDecl();

  // MSC-DAG: define          dllexport                void @"\01?staticDef@ExportClass@@SAXXZ"()
  // MSC-DAG: define weak_odr dllexport                void @"\01?staticInclass@ExportClass@@SAXXZ"()
  // MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDef@ExportClass@@SAXXZ"()
  // MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDecl@ExportClass@@SAXXZ"()
  // GNU-DAG: define          dllexport                void @_ZN11ExportClass9staticDefEv()
  // GNU-DAG: define weak_odr dllexport                void @_ZN11ExportClass13staticInclassEv()
  // GNU-DAG: define weak_odr dllexport                void @_ZN11ExportClass15staticInlineDefEv()
  // GNU-DAG: define weak_odr dllexport                void @_ZN11ExportClass16staticInlineDeclEv()
  static        void staticDef();
  static        void staticInclass() {}
  static        void staticInlineDef();
  static inline void staticInlineDecl();

  // M32-DAG: define          dllexport x86_thiscallcc void @"\01?protectedNormalDef@ExportClass@@IAEXXZ"(%struct.ExportClass* %this)
  // M64-DAG: define          dllexport                void @"\01?protectedNormalDef@ExportClass@@IEAAXXZ"(%struct.ExportClass* %this)
  // G32-DAG: define          dllexport x86_thiscallcc void @_ZN11ExportClass18protectedNormalDefEv(%struct.ExportClass* %this)
  // G64-DAG: define          dllexport                void @_ZN11ExportClass18protectedNormalDefEv(%struct.ExportClass* %this)
  // MSC-DAG: define          dllexport                void @"\01?protectedStaticDef@ExportClass@@KAXXZ"()
  // GNU-DAG: define          dllexport                void @_ZN11ExportClass18protectedStaticDefEv()
protected:
                void protectedNormalDef();
  static        void protectedStaticDef();

  // M32-DAG: define          dllexport x86_thiscallcc void @"\01?privateNormalDef@ExportClass@@AAEXXZ"(%struct.ExportClass* %this)
  // M64-DAG: define          dllexport                void @"\01?privateNormalDef@ExportClass@@AEAAXXZ"(%struct.ExportClass* %this)
  // G32-DAG: define          dllexport x86_thiscallcc void @_ZN11ExportClass16privateNormalDefEv(%struct.ExportClass* %this)
  // G64-DAG: define          dllexport                void @_ZN11ExportClass16privateNormalDefEv(%struct.ExportClass* %this)
  // MSC-DAG: define          dllexport                void @"\01?privateStaticDef@ExportClass@@CAXXZ"()
  // GNU-DAG: define          dllexport                void @_ZN11ExportClass16privateStaticDefEv()
private:
                void privateNormalDef();
  static        void privateStaticDef();

public:

  // MSC-DAG: @"\01?StaticField@ExportClass@@2HA"               = dllexport global i32 1, align 4
  // MSC-DAG: @"\01?StaticConstField@ExportClass@@2HB"          = dllexport constant i32 1, align 4
  // MSC-DAG: @"\01?StaticConstFieldEqualInit@ExportClass@@2HB" = weak_odr dllexport constant i32 1, align 4
  // MSC-DAG: @"\01?StaticConstFieldBraceInit@ExportClass@@2HB" = weak_odr dllexport constant i32 1, align 4
  // MSC-DAG: @"\01?ConstexprField@ExportClass@@2HB"            = weak_odr dllexport constant i32 1, align 4
  // GNU-DAG: @_ZN11ExportClass11StaticFieldE                   = dllexport global i32 1, align 4
  // GNU-DAG: @_ZN11ExportClass16StaticConstFieldE              = dllexport constant i32 1, align 4
  // GNU-DAG: @_ZN11ExportClass25StaticConstFieldEqualInitE     = dllexport constant i32 1, align 4
  // GNU-DAG: @_ZN11ExportClass25StaticConstFieldBraceInitE     = dllexport constant i32 1, align 4
  // GNU-DAG: @_ZN11ExportClass14ConstexprFieldE                = dllexport constant i32 1, align 4
                int  Field;
  static        int  StaticField;
  static const  int  StaticConstField;
  static const  int  StaticConstFieldEqualInit = 1;
  static const  int  StaticConstFieldBraceInit{1};
  constexpr static int ConstexprField = 1;
};

       void ExportClass::Nested::normalDef() {}
       void ExportClass::ExportNested::normalDef() {}
       void ExportClass::normalDef() {}
inline void ExportClass::normalInlineDef() {}
       void ExportClass::normalInlineDecl() {}
       void ExportClass::staticDef() {}
inline void ExportClass::staticInlineDef() {}
       void ExportClass::staticInlineDecl() {}
       void ExportClass::protectedNormalDef() {}
       void ExportClass::protectedStaticDef() {}
       void ExportClass::privateNormalDef() {}
       void ExportClass::privateStaticDef() {}

       int  ExportClass::StaticField = 1;
const  int  ExportClass::StaticConstField = 1;
const  int  ExportClass::StaticConstFieldEqualInit;
const  int  ExportClass::StaticConstFieldBraceInit;
constexpr int ExportClass::ConstexprField;


// Export class with special member functions.
struct __declspec(dllexport) ExportClassSpecials {
  // M32-DAG: define dllexport x86_thiscallcc %struct.ExportClassSpecials* @"\01??0ExportClassSpecials@@QAE@XZ"(%struct.ExportClassSpecials* returned %this)
  // M64-DAG: define dllexport                %struct.ExportClassSpecials* @"\01??0ExportClassSpecials@@QEAA@XZ"(%struct.ExportClassSpecials* returned %this)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN19ExportClassSpecialsC1Ev(%struct.ExportClassSpecials* %this)
  // G64-DAG: define dllexport                void @_ZN19ExportClassSpecialsC1Ev(%struct.ExportClassSpecials* %this)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN19ExportClassSpecialsC2Ev(%struct.ExportClassSpecials* %this)
  // G64-DAG: define dllexport                void @_ZN19ExportClassSpecialsC2Ev(%struct.ExportClassSpecials* %this)
  ExportClassSpecials();

  // M32-DAG: define dllexport x86_thiscallcc void @"\01??1ExportClassSpecials@@QAE@XZ"(%struct.ExportClassSpecials* %this)
  // M64-DAG: define dllexport                void @"\01??1ExportClassSpecials@@QEAA@XZ"(%struct.ExportClassSpecials* %this)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN19ExportClassSpecialsD1Ev(%struct.ExportClassSpecials* %this)
  // G64-DAG: define dllexport                void @_ZN19ExportClassSpecialsD1Ev(%struct.ExportClassSpecials* %this)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN19ExportClassSpecialsD2Ev(%struct.ExportClassSpecials* %this)
  // G64-DAG: define dllexport                void @_ZN19ExportClassSpecialsD2Ev(%struct.ExportClassSpecials* %this)
  ~ExportClassSpecials();

  // M32-DAG: define dllexport x86_thiscallcc %struct.ExportClassSpecials* @"\01??0ExportClassSpecials@@QAE@ABU0@@Z"(%struct.ExportClassSpecials* returned %this, %struct.ExportClassSpecials*)
  // M64-DAG: define dllexport                %struct.ExportClassSpecials* @"\01??0ExportClassSpecials@@QEAA@AEBU0@@Z"(%struct.ExportClassSpecials* returned %this, %struct.ExportClassSpecials*)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN19ExportClassSpecialsC1ERKS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G64-DAG: define dllexport                void @_ZN19ExportClassSpecialsC1ERKS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN19ExportClassSpecialsC2ERKS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G64-DAG: define dllexport                void @_ZN19ExportClassSpecialsC2ERKS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  ExportClassSpecials(const ExportClassSpecials&);

  // M32-DAG: define dllexport x86_thiscallcc %struct.ExportClassSpecials* @"\01??4ExportClassSpecials@@QAEAAU0@ABU0@@Z"(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // M64-DAG: define dllexport                %struct.ExportClassSpecials* @"\01??4ExportClassSpecials@@QEAAAEAU0@AEBU0@@Z"(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G32-DAG: define dllexport x86_thiscallcc %struct.ExportClassSpecials* @_ZN19ExportClassSpecialsaSERKS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G64-DAG: define dllexport                %struct.ExportClassSpecials* @_ZN19ExportClassSpecialsaSERKS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  ExportClassSpecials& operator=(const ExportClassSpecials&);

  // M32-DAG: define dllexport x86_thiscallcc %struct.ExportClassSpecials* @"\01??0ExportClassSpecials@@QAE@$$QAU0@@Z"(%struct.ExportClassSpecials* returned %this, %struct.ExportClassSpecials*)
  // M64-DAG: define dllexport                %struct.ExportClassSpecials* @"\01??0ExportClassSpecials@@QEAA@$$QEAU0@@Z"(%struct.ExportClassSpecials* returned %this, %struct.ExportClassSpecials*)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN19ExportClassSpecialsC1EOS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G64-DAG: define dllexport                void @_ZN19ExportClassSpecialsC1EOS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN19ExportClassSpecialsC2EOS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G64-DAG: define dllexport                void @_ZN19ExportClassSpecialsC2EOS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  ExportClassSpecials(ExportClassSpecials&&);

  // M32-DAG: define dllexport x86_thiscallcc %struct.ExportClassSpecials* @"\01??4ExportClassSpecials@@QAEAAU0@$$QAU0@@Z"(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // M64-DAG: define dllexport                %struct.ExportClassSpecials* @"\01??4ExportClassSpecials@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G32-DAG: define dllexport x86_thiscallcc %struct.ExportClassSpecials* @_ZN19ExportClassSpecialsaSEOS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  // G64-DAG: define dllexport                %struct.ExportClassSpecials* @_ZN19ExportClassSpecialsaSEOS_(%struct.ExportClassSpecials* %this, %struct.ExportClassSpecials*)
  ExportClassSpecials& operator=(ExportClassSpecials&&);
};
ExportClassSpecials::ExportClassSpecials() {}
ExportClassSpecials::~ExportClassSpecials() {}
ExportClassSpecials::ExportClassSpecials(const ExportClassSpecials&) {}
ExportClassSpecials& ExportClassSpecials::operator=(const ExportClassSpecials&) { return *this; }
ExportClassSpecials::ExportClassSpecials(ExportClassSpecials&&) {}
ExportClassSpecials& ExportClassSpecials::operator=(ExportClassSpecials&&) { return *this; }


// Implicitly defined trivial special member functions are not exported.
// MSC-NOT-FIXME: ExportTrivialSpecials
// GNU-NOT-FIXME: ExportTrivialSpecials
struct __declspec(dllexport) ExportTrivialSpecials {
};


// Implicitly defined non-trivial special member functions are exported.
//
// Default constructor
// M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportComplexSpecials* @"\01??0ExportComplexSpecials@@QAE@XZ"(%struct.ExportComplexSpecials* returned %this)
// M64-DAG: define weak_odr dllexport                %struct.ExportComplexSpecials* @"\01??0ExportComplexSpecials@@QEAA@XZ"(%struct.ExportComplexSpecials* returned %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN21ExportComplexSpecialsC1Ev(%struct.ExportComplexSpecials* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN21ExportComplexSpecialsC1Ev(%struct.ExportComplexSpecials* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN21ExportComplexSpecialsC2Ev(%struct.ExportComplexSpecials* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN21ExportComplexSpecialsC2Ev(%struct.ExportComplexSpecials* %this)
//
// Destructor
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01??1ExportComplexSpecials@@QAE@XZ"(%struct.ExportComplexSpecials* %this)
// M64-DAG: define weak_odr dllexport                void @"\01??1ExportComplexSpecials@@QEAA@XZ"(%struct.ExportComplexSpecials* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN21ExportComplexSpecialsD1Ev(%struct.ExportComplexSpecials* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN21ExportComplexSpecialsD1Ev(%struct.ExportComplexSpecials* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN21ExportComplexSpecialsD2Ev(%struct.ExportComplexSpecials* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN21ExportComplexSpecialsD2Ev(%struct.ExportComplexSpecials* %this)
//
// Copy constructor
// M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportComplexSpecials* @"\01??0ExportComplexSpecials@@QAE@ABU0@@Z"(%struct.ExportComplexSpecials* returned %this, %struct.ExportComplexSpecials*)
// M64-DAG: define weak_odr dllexport                %struct.ExportComplexSpecials* @"\01??0ExportComplexSpecials@@QEAA@AEBU0@@Z"(%struct.ExportComplexSpecials* returned %this, %struct.ExportComplexSpecials*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN21ExportComplexSpecialsC1ERKS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G64-DAG: define weak_odr dllexport                void @_ZN21ExportComplexSpecialsC1ERKS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN21ExportComplexSpecialsC2ERKS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G64-DAG: define weak_odr dllexport                void @_ZN21ExportComplexSpecialsC2ERKS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
//
// Copy assignment
// M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportComplexSpecials* @"\01??4ExportComplexSpecials@@QAEAAU0@ABU0@@Z"(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// M64-DAG: define weak_odr dllexport                %struct.ExportComplexSpecials* @"\01??4ExportComplexSpecials@@QEAAAEAU0@AEBU0@@Z"(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportComplexSpecials* @_ZN21ExportComplexSpecialsaSERKS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G64-DAG: define weak_odr dllexport                %struct.ExportComplexSpecials* @_ZN21ExportComplexSpecialsaSERKS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
//
// Move constructor
// M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportComplexSpecials* @"\01??0ExportComplexSpecials@@QAE@$$QAU0@@Z"(%struct.ExportComplexSpecials* returned %this, %struct.ExportComplexSpecials*)
// M64-DAG: define weak_odr dllexport                %struct.ExportComplexSpecials* @"\01??0ExportComplexSpecials@@QEAA@$$QEAU0@@Z"(%struct.ExportComplexSpecials* returned %this, %struct.ExportComplexSpecials*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN21ExportComplexSpecialsC1EOS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G64-DAG: define weak_odr dllexport                void @_ZN21ExportComplexSpecialsC1EOS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN21ExportComplexSpecialsC2EOS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G64-DAG: define weak_odr dllexport                void @_ZN21ExportComplexSpecialsC2EOS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
//
// Move assignment
// M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportComplexSpecials* @"\01??4ExportComplexSpecials@@QAEAAU0@$$QAU0@@Z"(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// M64-DAG: define weak_odr dllexport                %struct.ExportComplexSpecials* @"\01??4ExportComplexSpecials@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportComplexSpecials* @_ZN21ExportComplexSpecialsaSEOS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
// G64-DAG: define weak_odr dllexport                %struct.ExportComplexSpecials* @_ZN21ExportComplexSpecialsaSEOS_(%struct.ExportComplexSpecials* %this, %struct.ExportComplexSpecials*)
struct __declspec(dllexport) ExportComplexSpecials {
  ForceNonTrivial v;
};


// Export class with inline special member functions.
struct __declspec(dllexport) ExportClassInlineSpecials {
  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassInlineSpecials* @"\01??0ExportClassInlineSpecials@@QAE@XZ"(%struct.ExportClassInlineSpecials* returned %this)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassInlineSpecials* @"\01??0ExportClassInlineSpecials@@QEAA@XZ"(%struct.ExportClassInlineSpecials* returned %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN25ExportClassInlineSpecialsC1Ev(%struct.ExportClassInlineSpecials* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN25ExportClassInlineSpecialsC1Ev(%struct.ExportClassInlineSpecials* %this)
  ExportClassInlineSpecials() {}

  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01??1ExportClassInlineSpecials@@QAE@XZ"(%struct.ExportClassInlineSpecials* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01??1ExportClassInlineSpecials@@QEAA@XZ"(%struct.ExportClassInlineSpecials* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN25ExportClassInlineSpecialsD1Ev(%struct.ExportClassInlineSpecials* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN25ExportClassInlineSpecialsD1Ev(%struct.ExportClassInlineSpecials* %this)
  ~ExportClassInlineSpecials() {}

  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassInlineSpecials* @"\01??0ExportClassInlineSpecials@@QAE@ABU0@@Z"(%struct.ExportClassInlineSpecials* returned %this, %struct.ExportClassInlineSpecials*)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassInlineSpecials* @"\01??0ExportClassInlineSpecials@@QEAA@AEBU0@@Z"(%struct.ExportClassInlineSpecials* returned %this, %struct.ExportClassInlineSpecials*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN25ExportClassInlineSpecialsC1ERKS_(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  // G64-DAG: define weak_odr dllexport                void @_ZN25ExportClassInlineSpecialsC1ERKS_(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  inline ExportClassInlineSpecials(const ExportClassInlineSpecials&);

  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassInlineSpecials* @"\01??4ExportClassInlineSpecials@@QAEAAU0@ABU0@@Z"(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassInlineSpecials* @"\01??4ExportClassInlineSpecials@@QEAAAEAU0@AEBU0@@Z"(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassInlineSpecials* @_ZN25ExportClassInlineSpecialsaSERKS_(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  // G64-DAG: define weak_odr dllexport                %struct.ExportClassInlineSpecials* @_ZN25ExportClassInlineSpecialsaSERKS_(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  ExportClassInlineSpecials& operator=(const ExportClassInlineSpecials&);

  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassInlineSpecials* @"\01??0ExportClassInlineSpecials@@QAE@$$QAU0@@Z"(%struct.ExportClassInlineSpecials* returned %this, %struct.ExportClassInlineSpecials*)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassInlineSpecials* @"\01??0ExportClassInlineSpecials@@QEAA@$$QEAU0@@Z"(%struct.ExportClassInlineSpecials* returned %this, %struct.ExportClassInlineSpecials*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN25ExportClassInlineSpecialsC1EOS_(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  // G64-DAG: define weak_odr dllexport                void @_ZN25ExportClassInlineSpecialsC1EOS_(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  ExportClassInlineSpecials(ExportClassInlineSpecials&&) {}

  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassInlineSpecials* @"\01??4ExportClassInlineSpecials@@QAEAAU0@$$QAU0@@Z"(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassInlineSpecials* @"\01??4ExportClassInlineSpecials@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassInlineSpecials* @_ZN25ExportClassInlineSpecialsaSEOS_(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  // G64-DAG: define weak_odr dllexport                %struct.ExportClassInlineSpecials* @_ZN25ExportClassInlineSpecialsaSEOS_(%struct.ExportClassInlineSpecials* %this, %struct.ExportClassInlineSpecials*)
  ExportClassInlineSpecials& operator=(ExportClassInlineSpecials&&) { return *this; }
};
ExportClassInlineSpecials::ExportClassInlineSpecials(const ExportClassInlineSpecials&) {}
inline ExportClassInlineSpecials& ExportClassInlineSpecials::operator=(const ExportClassInlineSpecials&) { return *this; }


// Export class with deleted member functions.
// MSC-NOT-FIXME: ExportClassDeleted
// GNU-NOT-FIXME: ExportClassDeleted
struct __declspec(dllexport) ExportClassDeleted {
  ExportClassDeleted() = delete;
  ~ExportClassDeleted() = delete;
  ExportClassDeleted(const ExportClassDeleted&) = delete;
  ExportClassDeleted& operator=(const ExportClassDeleted&) = delete;
  ExportClassDeleted(ExportClassDeleted&&) = delete;
  ExportClassDeleted& operator=(ExportClassDeleted&&) = delete;
  void deleted() = delete;
};


// Export class with defaulted special member functions.
struct __declspec(dllexport) ExportClassDefaulted {
  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaulted* @"\01??0ExportClassDefaulted@@QAE@XZ"(%struct.ExportClassDefaulted* returned %this)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassDefaulted* @"\01??0ExportClassDefaulted@@QEAA@XZ"(%struct.ExportClassDefaulted* returned %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN20ExportClassDefaultedC1Ev(%struct.ExportClassDefaulted* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN20ExportClassDefaultedC1Ev(%struct.ExportClassDefaulted* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN20ExportClassDefaultedC2Ev(%struct.ExportClassDefaulted* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN20ExportClassDefaultedC2Ev(%struct.ExportClassDefaulted* %this)
  ExportClassDefaulted() = default;

  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01??1ExportClassDefaulted@@QAE@XZ"(%struct.ExportClassDefaulted* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01??1ExportClassDefaulted@@QEAA@XZ"(%struct.ExportClassDefaulted* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN20ExportClassDefaultedD1Ev(%struct.ExportClassDefaulted* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN20ExportClassDefaultedD1Ev(%struct.ExportClassDefaulted* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN20ExportClassDefaultedD2Ev(%struct.ExportClassDefaulted* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN20ExportClassDefaultedD2Ev(%struct.ExportClassDefaulted* %this)
  ~ExportClassDefaulted() = default;

  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaulted* @"\01??0ExportClassDefaulted@@QAE@ABU0@@Z"(%struct.ExportClassDefaulted* returned %this, %struct.ExportClassDefaulted*)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassDefaulted* @"\01??0ExportClassDefaulted@@QEAA@AEBU0@@Z"(%struct.ExportClassDefaulted* returned %this, %struct.ExportClassDefaulted*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN20ExportClassDefaultedC1ERKS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G64-DAG: define weak_odr dllexport                void @_ZN20ExportClassDefaultedC1ERKS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN20ExportClassDefaultedC2ERKS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G64-DAG: define weak_odr dllexport                void @_ZN20ExportClassDefaultedC2ERKS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  ExportClassDefaulted(const ExportClassDefaulted&) = default;

  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaulted* @"\01??4ExportClassDefaulted@@QAEAAU0@ABU0@@Z"(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassDefaulted* @"\01??4ExportClassDefaulted@@QEAAAEAU0@AEBU0@@Z"(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaulted* @_ZN20ExportClassDefaultedaSERKS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G64-DAG: define weak_odr dllexport                %struct.ExportClassDefaulted* @_ZN20ExportClassDefaultedaSERKS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  ExportClassDefaulted& operator=(const ExportClassDefaulted&) = default;

  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaulted* @"\01??0ExportClassDefaulted@@QAE@$$QAU0@@Z"(%struct.ExportClassDefaulted* returned %this, %struct.ExportClassDefaulted*)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassDefaulted* @"\01??0ExportClassDefaulted@@QEAA@$$QEAU0@@Z"(%struct.ExportClassDefaulted* returned %this, %struct.ExportClassDefaulted*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN20ExportClassDefaultedC1EOS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G64-DAG: define weak_odr dllexport                void @_ZN20ExportClassDefaultedC1EOS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN20ExportClassDefaultedC2EOS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G64-DAG: define weak_odr dllexport                void @_ZN20ExportClassDefaultedC2EOS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  ExportClassDefaulted(ExportClassDefaulted&&) = default;

  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaulted* @"\01??4ExportClassDefaulted@@QAEAAU0@$$QAU0@@Z"(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // M64-DAG: define weak_odr dllexport                %struct.ExportClassDefaulted* @"\01??4ExportClassDefaulted@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaulted* @_ZN20ExportClassDefaultedaSEOS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  // G64-DAG: define weak_odr dllexport                %struct.ExportClassDefaulted* @_ZN20ExportClassDefaultedaSEOS_(%struct.ExportClassDefaulted* %this, %struct.ExportClassDefaulted*)
  ExportClassDefaulted& operator=(ExportClassDefaulted&&) = default;
};


// Export class with defaulted member function definitions.
struct __declspec(dllexport) ExportClassDefaultedDefs {
  ExportClassDefaultedDefs();
  ~ExportClassDefaultedDefs();

  inline ExportClassDefaultedDefs(const ExportClassDefaultedDefs&);
  ExportClassDefaultedDefs& operator=(const ExportClassDefaultedDefs&);

  ExportClassDefaultedDefs(ExportClassDefaultedDefs&&);
  ExportClassDefaultedDefs& operator=(ExportClassDefaultedDefs&&);
};

// M32-DAG: define dllexport x86_thiscallcc %struct.ExportClassDefaultedDefs* @"\01??0ExportClassDefaultedDefs@@QAE@XZ"(%struct.ExportClassDefaultedDefs* returned %this)
// M64-DAG: define dllexport                %struct.ExportClassDefaultedDefs* @"\01??0ExportClassDefaultedDefs@@QEAA@XZ"(%struct.ExportClassDefaultedDefs* returned %this)
// G32-DAG: define dllexport x86_thiscallcc void @_ZN24ExportClassDefaultedDefsC1Ev(%struct.ExportClassDefaultedDefs* %this)
// G64-DAG: define dllexport                void @_ZN24ExportClassDefaultedDefsC1Ev(%struct.ExportClassDefaultedDefs* %this)
// G32-DAG: define dllexport x86_thiscallcc void @_ZN24ExportClassDefaultedDefsC2Ev(%struct.ExportClassDefaultedDefs* %this)
// G64-DAG: define dllexport                void @_ZN24ExportClassDefaultedDefsC2Ev(%struct.ExportClassDefaultedDefs* %this)
ExportClassDefaultedDefs::ExportClassDefaultedDefs() = default;

// M32-DAG: define dllexport x86_thiscallcc void @"\01??1ExportClassDefaultedDefs@@QAE@XZ"(%struct.ExportClassDefaultedDefs* %this)
// M64-DAG: define dllexport                void @"\01??1ExportClassDefaultedDefs@@QEAA@XZ"(%struct.ExportClassDefaultedDefs* %this)
// G32-DAG: define dllexport x86_thiscallcc void @_ZN24ExportClassDefaultedDefsD1Ev(%struct.ExportClassDefaultedDefs* %this)
// G64-DAG: define dllexport                void @_ZN24ExportClassDefaultedDefsD1Ev(%struct.ExportClassDefaultedDefs* %this)
// G32-DAG: define dllexport x86_thiscallcc void @_ZN24ExportClassDefaultedDefsD2Ev(%struct.ExportClassDefaultedDefs* %this)
// G64-DAG: define dllexport                void @_ZN24ExportClassDefaultedDefsD2Ev(%struct.ExportClassDefaultedDefs* %this)
ExportClassDefaultedDefs::~ExportClassDefaultedDefs() = default;

// M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaultedDefs* @"\01??0ExportClassDefaultedDefs@@QAE@ABU0@@Z"(%struct.ExportClassDefaultedDefs* returned %this, %struct.ExportClassDefaultedDefs*)
// M64-DAG: define weak_odr dllexport                %struct.ExportClassDefaultedDefs* @"\01??0ExportClassDefaultedDefs@@QEAA@AEBU0@@Z"(%struct.ExportClassDefaultedDefs* returned %this, %struct.ExportClassDefaultedDefs*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN24ExportClassDefaultedDefsC1ERKS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G64-DAG: define weak_odr dllexport                void @_ZN24ExportClassDefaultedDefsC1ERKS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN24ExportClassDefaultedDefsC2ERKS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G64-DAG: define weak_odr dllexport                void @_ZN24ExportClassDefaultedDefsC2ERKS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
ExportClassDefaultedDefs::ExportClassDefaultedDefs(const ExportClassDefaultedDefs&) = default;

// M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaultedDefs* @"\01??4ExportClassDefaultedDefs@@QAEAAU0@ABU0@@Z"(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// M64-DAG: define weak_odr dllexport                %struct.ExportClassDefaultedDefs* @"\01??4ExportClassDefaultedDefs@@QEAAAEAU0@AEBU0@@Z"(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportClassDefaultedDefs* @_ZN24ExportClassDefaultedDefsaSERKS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G64-DAG: define weak_odr dllexport                %struct.ExportClassDefaultedDefs* @_ZN24ExportClassDefaultedDefsaSERKS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
inline ExportClassDefaultedDefs& ExportClassDefaultedDefs::operator=(const ExportClassDefaultedDefs&) = default;

// M32-DAG: define dllexport x86_thiscallcc %struct.ExportClassDefaultedDefs* @"\01??0ExportClassDefaultedDefs@@QAE@$$QAU0@@Z"(%struct.ExportClassDefaultedDefs* returned %this, %struct.ExportClassDefaultedDefs*)
// M64-DAG: define dllexport                %struct.ExportClassDefaultedDefs* @"\01??0ExportClassDefaultedDefs@@QEAA@$$QEAU0@@Z"(%struct.ExportClassDefaultedDefs* returned %this, %struct.ExportClassDefaultedDefs*)
// G32-DAG: define dllexport x86_thiscallcc void @_ZN24ExportClassDefaultedDefsC1EOS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G64-DAG: define dllexport                void @_ZN24ExportClassDefaultedDefsC1EOS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G32-DAG: define dllexport x86_thiscallcc void @_ZN24ExportClassDefaultedDefsC2EOS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G64-DAG: define dllexport                void @_ZN24ExportClassDefaultedDefsC2EOS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
ExportClassDefaultedDefs::ExportClassDefaultedDefs(ExportClassDefaultedDefs&&) = default;

// M32-DAG: define dllexport x86_thiscallcc %struct.ExportClassDefaultedDefs* @"\01??4ExportClassDefaultedDefs@@QAEAAU0@$$QAU0@@Z"(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// M64-DAG: define dllexport                %struct.ExportClassDefaultedDefs* @"\01??4ExportClassDefaultedDefs@@QEAAAEAU0@$$QEAU0@@Z"(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G32-DAG: define dllexport x86_thiscallcc %struct.ExportClassDefaultedDefs* @_ZN24ExportClassDefaultedDefsaSEOS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
// G64-DAG: define dllexport                %struct.ExportClassDefaultedDefs* @_ZN24ExportClassDefaultedDefsaSEOS_(%struct.ExportClassDefaultedDefs* %this, %struct.ExportClassDefaultedDefs*)
ExportClassDefaultedDefs& ExportClassDefaultedDefs::operator=(ExportClassDefaultedDefs&&) = default;


// Export class with allocation functions.
struct __declspec(dllexport) ExportClassAlloc {
  void* operator new(__SIZE_TYPE__);
  void* operator new[](__SIZE_TYPE__);
  void operator delete(void*);
  void operator delete[](void*);
};

// M32-DAG: define dllexport i8* @"\01??2ExportClassAlloc@@SAPAXI@Z"(i32 %n)
// M64-DAG: define dllexport i8* @"\01??2ExportClassAlloc@@SAPEAX_K@Z"(i64 %n)
// G32-DAG: define dllexport i8* @_ZN16ExportClassAllocnwEj(i32 %n)
// G64-DAG: define dllexport i8* @_ZN16ExportClassAllocnwEy(i64 %n)
void* ExportClassAlloc::operator new(__SIZE_TYPE__ n) { return malloc(n); }

// M32-DAG: define dllexport i8* @"\01??_UExportClassAlloc@@SAPAXI@Z"(i32 %n)
// M64-DAG: define dllexport i8* @"\01??_UExportClassAlloc@@SAPEAX_K@Z"(i64 %n)
// G32-DAG: define dllexport i8* @_ZN16ExportClassAllocnaEj(i32 %n)
// G64-DAG: define dllexport i8* @_ZN16ExportClassAllocnaEy(i64 %n)
void* ExportClassAlloc::operator new[](__SIZE_TYPE__ n) { return malloc(n); }

// M32-DAG: define dllexport void @"\01??3ExportClassAlloc@@SAXPAX@Z"(i8* %p)
// M64-DAG: define dllexport void @"\01??3ExportClassAlloc@@SAXPEAX@Z"(i8* %p)
// G32-DAG: define dllexport void @_ZN16ExportClassAllocdlEPv(i8* %p)
// G64-DAG: define dllexport void @_ZN16ExportClassAllocdlEPv(i8* %p)
void ExportClassAlloc::operator delete(void* p) { free(p); }

// M32-DAG: define dllexport void @"\01??_VExportClassAlloc@@SAXPAX@Z"(i8* %p)
// M64-DAG: define dllexport void @"\01??_VExportClassAlloc@@SAXPEAX@Z"(i8* %p)
// G32-DAG: define dllexport void @_ZN16ExportClassAllocdaEPv(i8* %p)
// G64-DAG: define dllexport void @_ZN16ExportClassAllocdaEPv(i8* %p)
void ExportClassAlloc::operator delete[](void* p) { free(p); }


// NOP-DAG: define linkonce_odr dllexport void @_ZN7Attribs12AlwaysInlineEv({{[^\)]*}}) #[[AlwaysInlineExportGrp:[0-9]+]]
// NOP-DAG: define linkonce_odr dllexport void @_ZN7Attribs11NeverInlineEv({{[^\)]*}})  #[[NoInlineExportGrp:[0-9]+]]
struct __declspec(dllexport) Attribs {
  __attribute__((always_inline)) void AlwaysInline() {}
  __attribute__((noinline)) void NeverInline() {}
};


// Export dynamic class with key function.
//
// --- MS ABI ---
// MSC-DAG-FIXME: @"\01??_7ExportDynamicClass@@6B@"         = weak_odr dllexport unnamed_addr constant
//
// RTTI Type Desc., Base Class Desc., Base Class Array, Class Hierarchy Desc.,
// Complete Object Locator and type_info VTable are not exported.
// MSC-DAG: @"\01??_R0?AUExportDynamicClass@@@8"      = linkonce_odr global
// MSC-DAG: @"\01??_R1A@?0A@EA@ExportDynamicClass@@8" = linkonce_odr constant
// MSC-DAG: @"\01??_R2ExportDynamicClass@@8"          = linkonce_odr constant
// MSC-DAG: @"\01??_R3ExportDynamicClass@@8"          = linkonce_odr constant
// MSC-DAG: @"\01??_R4ExportDynamicClass@@6B@"        = linkonce_odr constant
// MSC-DAG: @"\01??_7type_info@@6B@"                  = external constant
//
// Scalar and vector deleting destructor
// M32-DAG: define linkonce_odr x86_thiscallcc void @"\01??_GExportDynamicClass@@UAEPAXI@Z"(%struct.ExportDynamicClass* %this, i32 %should_call_delete)
// M64-DAG: define linkonce_odr                void @"\01??_GExportDynamicClass@@UEAAPEAXI@Z"(%struct.ExportDynamicClass* %this, i32 %should_call_delete)
// ??_EExportDynamicClass@@UEAAPEAXI@Z
//
// --- Mingw ABI ---
// GNU-DAG: @_ZTV18ExportDynamicClass = dllexport unnamed_addr constant
// GNU-DAG: @_ZTI18ExportDynamicClass = dllexport constant
// GNU-DAG: @_ZTS18ExportDynamicClass = constant
struct __declspec(dllexport) ExportDynamicClass {
  // Default constructor
  // M32-DAG: define weak_odr dllexport x86_thiscallcc %struct.ExportDynamicClass* @"\01??0ExportDynamicClass@@QAE@XZ"(%struct.ExportDynamicClass* returned %this)
  // M64-DAG: define weak_odr dllexport                %struct.ExportDynamicClass* @"\01??0ExportDynamicClass@@QEAA@XZ"(%struct.ExportDynamicClass* returned %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN18ExportDynamicClassC1Ev(%struct.ExportDynamicClass* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN18ExportDynamicClassC1Ev(%struct.ExportDynamicClass* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN18ExportDynamicClassC2Ev(%struct.ExportDynamicClass* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN18ExportDynamicClassC2Ev(%struct.ExportDynamicClass* %this)

  // Copy constructor
  // M32-DAG-FIXME: define          dllexport x86_thiscallcc %struct.ExportDynamicClass* ??0ExportDynamicClass@@QAE@ABU0@@Z(%struct.ExportDynamicClass* returned %this, %struct.ExportDynamicClass*)
  // M32-DAG-FIXME: define          dllexport                %struct.ExportDynamicClass* ??0ExportDynamicClass@@QEAA@AEBU0@@Z(%struct.ExportDynamicClass* returned %this, %struct.ExportDynamicClass*)

  // Copy assignment
  // M32-DAG-FIXME: define          dllexport x86_thiscallcc %struct.ExportDynamicClass* ??4ExportDynamicClass@@QAEAAU0@ABU0@@Z(%struct.ExportDynamicClass* %this, %struct.ExportDynamicClass*)
  // M32-DAG-FIXME: define          dllexport                %struct.ExportDynamicClass* ??4ExportDynamicClass@@QEAAAEAU0@AEBU0@@Z(%struct.ExportDynamicClass* %this, %struct.ExportDynamicClass*)

  // M32-DAG: define dllexport x86_thiscallcc void @"\01??1ExportDynamicClass@@UAE@XZ"(%struct.ExportDynamicClass* %this)
  // M64-DAG: define dllexport                void @"\01??1ExportDynamicClass@@UEAA@XZ"(%struct.ExportDynamicClass* %this)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN18ExportDynamicClassD0Ev(%struct.ExportDynamicClass* %this)
  // G64-DAG: define dllexport                void @_ZN18ExportDynamicClassD0Ev(%struct.ExportDynamicClass* %this)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN18ExportDynamicClassD1Ev(%struct.ExportDynamicClass* %this)
  // G64-DAG: define dllexport                void @_ZN18ExportDynamicClassD1Ev(%struct.ExportDynamicClass* %this)
  // G32-DAG: define dllexport x86_thiscallcc void @_ZN18ExportDynamicClassD2Ev(%struct.ExportDynamicClass* %this)
  // G64-DAG: define dllexport                void @_ZN18ExportDynamicClassD2Ev(%struct.ExportDynamicClass* %this)
  virtual ~ExportDynamicClass();

  // M32-DAG: define          dllexport x86_thiscallcc void @"\01?virtualDef@ExportDynamicClass@@UAEXXZ"(%struct.ExportDynamicClass* %this)
  // M64-DAG: define          dllexport                void @"\01?virtualDef@ExportDynamicClass@@UEAAXXZ"(%struct.ExportDynamicClass* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?virtualInclass@ExportDynamicClass@@UAEXXZ"(%struct.ExportDynamicClass* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?virtualInclass@ExportDynamicClass@@UEAAXXZ"(%struct.ExportDynamicClass* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?virtualInlineDef@ExportDynamicClass@@UAEXXZ"(%struct.ExportDynamicClass* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?virtualInlineDef@ExportDynamicClass@@UEAAXXZ"(%struct.ExportDynamicClass* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?virtualInlineDecl@ExportDynamicClass@@UAEXXZ"(%struct.ExportDynamicClass* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?virtualInlineDecl@ExportDynamicClass@@UEAAXXZ"(%struct.ExportDynamicClass* %this)
  // G32-DAG: define          dllexport x86_thiscallcc void @_ZN18ExportDynamicClass10virtualDefEv(%struct.ExportDynamicClass* %this)
  // G64-DAG: define          dllexport                void @_ZN18ExportDynamicClass10virtualDefEv(%struct.ExportDynamicClass* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN18ExportDynamicClass14virtualInclassEv(%struct.ExportDynamicClass* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN18ExportDynamicClass14virtualInclassEv(%struct.ExportDynamicClass* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN18ExportDynamicClass16virtualInlineDefEv(%struct.ExportDynamicClass* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN18ExportDynamicClass16virtualInlineDefEv(%struct.ExportDynamicClass* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN18ExportDynamicClass17virtualInlineDeclEv(%struct.ExportDynamicClass* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN18ExportDynamicClass17virtualInlineDeclEv(%struct.ExportDynamicClass* %this)
  virtual        void virtualDef();
  virtual        void virtualInclass() {}
  virtual        void virtualInlineDef();
  virtual inline void virtualInlineDecl();
};

            ExportDynamicClass::~ExportDynamicClass() {}
       void ExportDynamicClass::virtualDef() {}
inline void ExportDynamicClass::virtualInlineDef() {}
       void ExportDynamicClass::virtualInlineDecl() {}


// Export dynamic class without key function.
//
// Note: vtable and rtti are not exported.
// NOP-DAG: @_ZTV13VirtualInline = linkonce_odr unnamed_addr constant {{.*}}]{{$}}
// NOP-DAG: @_ZTT13VirtualInline = linkonce_odr unnamed_addr constant {{.*}}]{{$}}
// NOP-DAG: @_ZTI13VirtualInline = linkonce_odr unnamed_addr constant {{.*}}}{{$}}
// NOP-DAG: @_ZTS13VirtualInline = linkonce_odr constant {{.*}}"{{$}}
// NOP-DAG: define linkonce_odr dllexport void      @_ZN13VirtualInlineC1Ev({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void      @_ZN13VirtualInlineC2Ev({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void      @_ZN13VirtualInlineC1ERKS_({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void      @_ZN13VirtualInlineC2ERKS_({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport {{[^ ]*}} @_ZN13VirtualInlineaSERKS_({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void      @_ZN13VirtualInlineD0Ev({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void      @_ZN13VirtualInlineD1Ev({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void      @_ZN13VirtualInlineD2Ev({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void      @_ZN13VirtualInline19publicVirtualInsideEv({{[^\)]*}})
//
// --- Mingw ABI ---
// GNU-DAG-FIXME: @_ZTV24ExportInlineDynamicClass = weak_odr dllexport unnamed_addr constant
// GNU-DAG-FIXME: @_ZTI24ExportInlineDynamicClass = weak_odr dllexport constant
// GNU-DAG-FIXME: @_ZTS24ExportInlineDynamicClass = linkonce_odr constant
struct __declspec(dllexport) ExportInlineDynamicClass {
  virtual ~ExportInlineDynamicClass() {}
  virtual void virtualInclass() {}
};


// Export dynamic class with virtual inheritance.
//
// NOP-DAG: @_ZTV12VirtualBase1 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: @_ZTI12VirtualBase1 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: @_ZTS12VirtualBase1 = dllexport constant {{.*}}
//
// NOP-DAG: @_ZTV12VirtualBase2 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: @_ZTT12VirtualBase2 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: @_ZTI12VirtualBase2 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: @_ZTS12VirtualBase2 = dllexport constant {{.*}}
// NOP-DAG: define dllexport void @_ZTv0_n{{12|24}}_N12VirtualBase2D0Ev({{[^\)]*}})
// NOP-DAG: define dllexport void @_ZTv0_n{{12|24}}_N12VirtualBase2D1Ev({{[^\)]*}})

// NOP-DAG: @_ZTV12VirtualBase3 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: @_ZTT12VirtualBase3 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: @_ZTI12VirtualBase3 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: @_ZTS12VirtualBase3 = dllexport constant {{.*}}
// NOP-DAG: @_ZTC12VirtualBase30_12VirtualBase2 = dllexport unnamed_addr constant {{.*}}
// NOP-DAG: define dllexport void @_ZTv0_n{{12|24}}_N12VirtualBase3D0Ev({{[^\)]*}})
// NOP-DAG: define dllexport void @_ZTv0_n{{12|24}}_N12VirtualBase3D1Ev({{[^\)]*}})
struct __declspec(dllexport) VirtualBase1 { virtual ~VirtualBase1(); };
struct __declspec(dllexport) VirtualBase2 : virtual VirtualBase1 { virtual ~VirtualBase2(); };
struct __declspec(dllexport) VirtualBase3 : virtual VirtualBase2 { virtual ~VirtualBase3(); };
VirtualBase1::~VirtualBase1() {}
VirtualBase2::~VirtualBase2() {}
VirtualBase3::~VirtualBase3() {}


// Export dynamic class without key function and with virtual inheritance.
//
// NOP-DAG: @_ZTV18VirtualInlineBase1 = linkonce_odr unnamed_addr constant {{.*}}]{{$}}
// NOP-DAG: @_ZTI18VirtualInlineBase1 = linkonce_odr unnamed_addr constant {{.*}}}{{$}}
// NOP-DAG: @_ZTS18VirtualInlineBase1 = linkonce_odr constant {{.*}}"{{$}}
//
// NOP-DAG: @_ZTV18VirtualInlineBase2 = linkonce_odr unnamed_addr constant {{.*}}]{{$}}
// NOP-DAG: @_ZTT18VirtualInlineBase2 = linkonce_odr unnamed_addr constant {{.*}}]{{$}}
// NOP-DAG: @_ZTI18VirtualInlineBase2 = linkonce_odr unnamed_addr constant {{.*}}}{{$}}
// NOP-DAG: @_ZTS18VirtualInlineBase2 = linkonce_odr constant {{.*}}"{{$}}
// NOP-DAG: define linkonce_odr dllexport void @_ZTv0_n{{12|24}}_N18VirtualInlineBase2D0Ev({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void @_ZTv0_n{{12|24}}_N18VirtualInlineBase2D1Ev({{[^\)]*}})
//
// NOP-DAG: @_ZTV18VirtualInlineBase3 = linkonce_odr unnamed_addr constant {{.*}}]{{$}}
// NOP-DAG: @_ZTT18VirtualInlineBase3 = linkonce_odr unnamed_addr constant {{.*}}]{{$}}
// NOP-DAG: @_ZTI18VirtualInlineBase3 = linkonce_odr unnamed_addr constant {{.*}}}{{$}}
// NOP-DAG: @_ZTS18VirtualInlineBase3 = linkonce_odr constant {{.*}}"{{$}}
// NOP-DAG: @_ZTC18VirtualInlineBase30_18VirtualInlineBase2 = linkonce_odr unnamed_addr constant {{.*}}]{{$}}
// NOP-DAG: define linkonce_odr dllexport void @_ZTv0_n{{12|24}}_N18VirtualInlineBase3D0Ev({{[^\)]*}})
// NOP-DAG: define linkonce_odr dllexport void @_ZTv0_n{{12|24}}_N18VirtualInlineBase3D1Ev({{[^\)]*}})
struct __declspec(dllexport) VirtualInlineBase1 { virtual ~VirtualInlineBase1() {} };
struct __declspec(dllexport) VirtualInlineBase2 : virtual VirtualInlineBase1 { virtual ~VirtualInlineBase2() {} };
struct __declspec(dllexport) VirtualInlineBase3 : virtual VirtualInlineBase2 { virtual ~VirtualInlineBase3() {} };


// NOP: attributes #[[AlwaysInlineExportGrp]] = { alwaysinline nounwind {{["\}]}}
// NOP: attributes #[[NoInlineExportGrp]]     = { noinline nounwind {{["\}]}}



//===----------------------------------------------------------------------===//
// Class templates
//===----------------------------------------------------------------------===//

// Exported class template.
template<typename T>
struct __declspec(dllexport) ExportClsTmpl {
                void normalDef();
                void normalInclass() {}
                void normalInlineDef();
         inline void normalInlineDecl();
  static        void staticDef();
  static        void staticInclass() {}
  static        void staticInlineDef();
  static inline void staticInlineDecl();

                int  Field;
  static        int  StaticField;
  static const  int  StaticConstField;
  static const  int  StaticConstFieldEqualInit = 1;
  static const  int  StaticConstFieldBraceInit{1};
  constexpr static int ConstexprField = 1;
};

template<typename T>        void ExportClsTmpl<T>::normalDef() {}
template<typename T> inline void ExportClsTmpl<T>::normalInlineDef() {}
template<typename T>        void ExportClsTmpl<T>::normalInlineDecl() {}
template<typename T>        void ExportClsTmpl<T>::staticDef() {}
template<typename T> inline void ExportClsTmpl<T>::staticInlineDef() {}
template<typename T>        void ExportClsTmpl<T>::staticInlineDecl() {}

template<typename T>        int  ExportClsTmpl<T>::StaticField = 1;
template<typename T> const  int  ExportClsTmpl<T>::StaticConstField = 1;
template<typename T> const  int  ExportClsTmpl<T>::StaticConstFieldEqualInit;
template<typename T> const  int  ExportClsTmpl<T>::StaticConstFieldBraceInit;
template<typename T> constexpr int ExportClsTmpl<T>::ConstexprField;


// Non-exported class template.
template<typename T>
struct ClassTmpl {
                void normalDef();
                void normalInclass() {}
                void normalInlineDef();
         inline void normalInlineDecl();
  static        void staticDef();
  static        void staticInclass() {}
  static        void staticInlineDef();
  static inline void staticInlineDecl();

                int  Field;
  static        int  StaticField;
  static const  int  StaticConstField;
  static const  int  StaticConstFieldEqualInit = 1;
  static const  int  StaticConstFieldBraceInit{1};
  constexpr static int ConstexprField = 1;
};

template<typename T>        void ClassTmpl<T>::normalDef() {}
template<typename T> inline void ClassTmpl<T>::normalInlineDef() {}
template<typename T>        void ClassTmpl<T>::normalInlineDecl() {}
template<typename T>        void ClassTmpl<T>::staticDef() {}
template<typename T> inline void ClassTmpl<T>::staticInlineDef() {}
template<typename T>        void ClassTmpl<T>::staticInlineDecl() {}

template<typename T>        int  ClassTmpl<T>::StaticField = 1;
template<typename T> const  int  ClassTmpl<T>::StaticConstField = 1;
template<typename T> const  int  ClassTmpl<T>::StaticConstFieldEqualInit;
template<typename T> const  int  ClassTmpl<T>::StaticConstFieldBraceInit;
template<typename T> constexpr int ClassTmpl<T>::ConstexprField;


// Export explicit instantiation declaration of an exported class template.
//
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalDef@?$ExportClsTmpl@UExplicitDecl_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalDef@?$ExportClsTmpl@UExplicitDecl_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInclass@?$ExportClsTmpl@UExplicitDecl_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInclass@?$ExportClsTmpl@UExplicitDecl_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDef@?$ExportClsTmpl@UExplicitDecl_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDef@?$ExportClsTmpl@UExplicitDecl_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDecl@?$ExportClsTmpl@UExplicitDecl_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDecl@?$ExportClsTmpl@UExplicitDecl_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE9normalDefEv(%struct.ExportClsTmpl* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE9normalDefEv(%struct.ExportClsTmpl* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE13normalInclassEv(%struct.ExportClsTmpl* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE13normalInclassEv(%struct.ExportClsTmpl* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE15normalInlineDefEv(%struct.ExportClsTmpl* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE15normalInlineDefEv(%struct.ExportClsTmpl* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE16normalInlineDeclEv(%struct.ExportClsTmpl* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE16normalInlineDeclEv(%struct.ExportClsTmpl* %this)
//
// MSC-DAG: define weak_odr dllexport                void @"\01?staticDef@?$ExportClsTmpl@UExplicitDecl_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInclass@?$ExportClsTmpl@UExplicitDecl_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDef@?$ExportClsTmpl@UExplicitDecl_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDecl@?$ExportClsTmpl@UExplicitDecl_Exported@@@@SAXXZ"()
// GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE9staticDefEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE13staticInclassEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE15staticInlineDefEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE16staticInlineDeclEv()
//
// MSC-DAG: @"\01?StaticField@?$ExportClsTmpl@UExplicitDecl_Exported@@@@2HA"               = weak_odr dllexport global i32 1, align 4
// MSC-DAG: @"\01?StaticConstField@?$ExportClsTmpl@UExplicitDecl_Exported@@@@2HB"          = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ExportClsTmpl@UExplicitDecl_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ExportClsTmpl@UExplicitDecl_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?ConstexprField@?$ExportClsTmpl@UExplicitDecl_Exported@@@@2HB"            = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE11StaticFieldE                     = weak_odr dllexport global i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE16StaticConstFieldE                = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE25StaticConstFieldEqualInitE       = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE25StaticConstFieldBraceInitE       = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitDecl_ExportedE14ConstexprFieldE                  = weak_odr dllexport constant i32 1, align 4
extern template struct ExportClsTmpl<ExplicitDecl_Exported>;
       template struct ExportClsTmpl<ExplicitDecl_Exported>;


// Export explicit instantiation definition of an exported class template.
//
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalDef@?$ExportClsTmpl@UExplicitInst_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl.0* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalDef@?$ExportClsTmpl@UExplicitInst_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl.0* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInclass@?$ExportClsTmpl@UExplicitInst_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl.0* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInclass@?$ExportClsTmpl@UExplicitInst_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl.0* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDef@?$ExportClsTmpl@UExplicitInst_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl.0* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDef@?$ExportClsTmpl@UExplicitInst_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl.0* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDecl@?$ExportClsTmpl@UExplicitInst_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl.0* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDecl@?$ExportClsTmpl@UExplicitInst_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl.0* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE9normalDefEv(%struct.ExportClsTmpl.0* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE9normalDefEv(%struct.ExportClsTmpl.0* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE13normalInclassEv(%struct.ExportClsTmpl.0* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE13normalInclassEv(%struct.ExportClsTmpl.0* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE15normalInlineDefEv(%struct.ExportClsTmpl.0* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE15normalInlineDefEv(%struct.ExportClsTmpl.0* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE16normalInlineDeclEv(%struct.ExportClsTmpl.0* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE16normalInlineDeclEv(%struct.ExportClsTmpl.0* %this)
//
// MSC-DAG: define weak_odr dllexport                void @"\01?staticDef@?$ExportClsTmpl@UExplicitInst_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInclass@?$ExportClsTmpl@UExplicitInst_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDef@?$ExportClsTmpl@UExplicitInst_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDecl@?$ExportClsTmpl@UExplicitInst_Exported@@@@SAXXZ"()
// GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE9staticDefEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE13staticInclassEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE15staticInlineDefEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitInst_ExportedE16staticInlineDeclEv()
//
// MSC-DAG: @"\01?StaticField@?$ExportClsTmpl@UExplicitInst_Exported@@@@2HA"               = weak_odr dllexport global i32 1, align 4
// MSC-DAG: @"\01?StaticConstField@?$ExportClsTmpl@UExplicitInst_Exported@@@@2HB"          = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ExportClsTmpl@UExplicitInst_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ExportClsTmpl@UExplicitInst_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?ConstexprField@?$ExportClsTmpl@UExplicitInst_Exported@@@@2HB"            = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitInst_ExportedE11StaticFieldE                     = weak_odr dllexport global i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitInst_ExportedE16StaticConstFieldE                = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitInst_ExportedE25StaticConstFieldEqualInitE       = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitInst_ExportedE25StaticConstFieldBraceInitE       = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN13ExportClsTmplI21ExplicitInst_ExportedE14ConstexprFieldE                  = weak_odr dllexport constant i32 1, align 4
template struct ExportClsTmpl<ExplicitInst_Exported>;


// Export specialization of an exported class template.
template<>
struct __declspec(dllexport) ExportClsTmpl<ExplicitSpec_Exported> {
  // M32-DAG: define          dllexport x86_thiscallcc void @"\01?normalDef@?$ExportClsTmpl@UExplicitSpec_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl.1* %this)
  // M64-DAG: define          dllexport                void @"\01?normalDef@?$ExportClsTmpl@UExplicitSpec_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl.1* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInclass@?$ExportClsTmpl@UExplicitSpec_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl.1* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?normalInclass@?$ExportClsTmpl@UExplicitSpec_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl.1* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDef@?$ExportClsTmpl@UExplicitSpec_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl.1* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDef@?$ExportClsTmpl@UExplicitSpec_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl.1* %this)
  // M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDecl@?$ExportClsTmpl@UExplicitSpec_Exported@@@@QAEXXZ"(%struct.ExportClsTmpl.1* %this)
  // M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDecl@?$ExportClsTmpl@UExplicitSpec_Exported@@@@QEAAXXZ"(%struct.ExportClsTmpl.1* %this)
  // G32-DAG: define          dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE9normalDefEv(%struct.ExportClsTmpl.1* %this)
  // G64-DAG: define          dllexport                void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE9normalDefEv(%struct.ExportClsTmpl.1* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE13normalInclassEv(%struct.ExportClsTmpl.1* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE13normalInclassEv(%struct.ExportClsTmpl.1* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE15normalInlineDefEv(%struct.ExportClsTmpl.1* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE15normalInlineDefEv(%struct.ExportClsTmpl.1* %this)
  // G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE16normalInlineDeclEv(%struct.ExportClsTmpl.1* %this)
  // G64-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE16normalInlineDeclEv(%struct.ExportClsTmpl.1* %this)
                 void normalDef();
                 void normalInclass() {}
                 void normalInlineDef();
          inline void normalInlineDecl();

  // MSC-DAG: define          dllexport                void @"\01?staticDef@?$ExportClsTmpl@UExplicitSpec_Exported@@@@SAXXZ"()
  // MSC-DAG: define weak_odr dllexport                void @"\01?staticInclass@?$ExportClsTmpl@UExplicitSpec_Exported@@@@SAXXZ"()
  // MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDef@?$ExportClsTmpl@UExplicitSpec_Exported@@@@SAXXZ"()
  // MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDecl@?$ExportClsTmpl@UExplicitSpec_Exported@@@@SAXXZ"()
  // GNU-DAG: define          dllexport                void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE9staticDefEv()
  // GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE13staticInclassEv()
  // GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE15staticInlineDefEv()
  // GNU-DAG: define weak_odr dllexport                void @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE16staticInlineDeclEv()
  static         void staticDef();
  static         void staticInclass() {}
  static         void staticInlineDef();
  static  inline void staticInlineDecl();

  // MSC-DAG: @"\01?StaticField@?$ExportClsTmpl@UExplicitSpec_Exported@@@@2HA"               = dllexport global i32 1, align 4
  // MSC-DAG: @"\01?StaticConstField@?$ExportClsTmpl@UExplicitSpec_Exported@@@@2HB"          = dllexport constant i32 1, align 4
  // MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ExportClsTmpl@UExplicitSpec_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
  // MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ExportClsTmpl@UExplicitSpec_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
  // MSC-DAG: @"\01?ConstexprField@?$ExportClsTmpl@UExplicitSpec_Exported@@@@2HB"            = weak_odr dllexport constant i32 1, align 4
  // GNU-DAG: @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE11StaticFieldE                     = dllexport global i32 1, align 4
  // GNU-DAG: @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE16StaticConstFieldE                = dllexport constant i32 1, align 4
  // GNU-DAG: @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE25StaticConstFieldEqualInitE       = dllexport constant i32 1, align 4
  // GNU-DAG: @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE25StaticConstFieldBraceInitE       = dllexport constant i32 1, align 4
  // GNU-DAG: @_ZN13ExportClsTmplI21ExplicitSpec_ExportedE14ConstexprFieldE                  = dllexport constant i32 1, align 4
                int  Field;
  static        int  StaticField;
  static const  int  StaticConstField;
  static const  int  StaticConstFieldEqualInit = 1;
  static const  int  StaticConstFieldBraceInit{1};
  constexpr static int ConstexprField = 1;
};

       void ExportClsTmpl<ExplicitSpec_Exported>::normalDef() {}
inline void ExportClsTmpl<ExplicitSpec_Exported>::normalInlineDef() {}
       void ExportClsTmpl<ExplicitSpec_Exported>::normalInlineDecl() {}
       void ExportClsTmpl<ExplicitSpec_Exported>::staticDef() {}
inline void ExportClsTmpl<ExplicitSpec_Exported>::staticInlineDef() {}
       void ExportClsTmpl<ExplicitSpec_Exported>::staticInlineDecl() {}

       int  ExportClsTmpl<ExplicitSpec_Exported>::StaticField = 1;
const  int  ExportClsTmpl<ExplicitSpec_Exported>::StaticConstField = 1;
const  int  ExportClsTmpl<ExplicitSpec_Exported>::StaticConstFieldEqualInit;
const  int  ExportClsTmpl<ExplicitSpec_Exported>::StaticConstFieldBraceInit;
constexpr int ExportClsTmpl<ExplicitSpec_Exported>::ConstexprField;


// Not exporting specialization of an exported class template without explicit
// dllexport.
// M32-DAG: define x86_thiscallcc void @"\01?normalDef@?$ExportClsTmpl@UExplicitSpec_NotExported@@@@QAEXXZ"(%struct.ExportClsTmpl.2* %this)
// M64-DAG: define                void @"\01?normalDef@?$ExportClsTmpl@UExplicitSpec_NotExported@@@@QEAAXXZ"(%struct.ExportClsTmpl.2* %this)
// G32-DAG: define x86_thiscallcc void @_ZN13ExportClsTmplI24ExplicitSpec_NotExportedE9normalDefEv(%struct.ExportClsTmpl.2* %this)
// G64-DAG: define                void @_ZN13ExportClsTmplI24ExplicitSpec_NotExportedE9normalDefEv(%struct.ExportClsTmpl.2* %this)
template<>
struct ExportClsTmpl<ExplicitSpec_NotExported> {
  void normalDef();
};
void ExportClsTmpl<ExplicitSpec_NotExported>::normalDef() {}


// Export explicit instantiation declaration of a non-exported class template.
//
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalDef@?$ClassTmpl@UExplicitDecl_Exported@@@@QAEXXZ"(%struct.ClassTmpl* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalDef@?$ClassTmpl@UExplicitDecl_Exported@@@@QEAAXXZ"(%struct.ClassTmpl* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInclass@?$ClassTmpl@UExplicitDecl_Exported@@@@QAEXXZ"(%struct.ClassTmpl* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInclass@?$ClassTmpl@UExplicitDecl_Exported@@@@QEAAXXZ"(%struct.ClassTmpl* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDef@?$ClassTmpl@UExplicitDecl_Exported@@@@QAEXXZ"(%struct.ClassTmpl* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDef@?$ClassTmpl@UExplicitDecl_Exported@@@@QEAAXXZ"(%struct.ClassTmpl* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDecl@?$ClassTmpl@UExplicitDecl_Exported@@@@QAEXXZ"(%struct.ClassTmpl* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDecl@?$ClassTmpl@UExplicitDecl_Exported@@@@QEAAXXZ"(%struct.ClassTmpl* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ExportedE9normalDefEv(%struct.ClassTmpl* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitDecl_ExportedE9normalDefEv(%struct.ClassTmpl* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ExportedE13normalInclassEv(%struct.ClassTmpl* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitDecl_ExportedE13normalInclassEv(%struct.ClassTmpl* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ExportedE15normalInlineDefEv(%struct.ClassTmpl* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitDecl_ExportedE15normalInlineDefEv(%struct.ClassTmpl* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitDecl_ExportedE16normalInlineDeclEv(%struct.ClassTmpl* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitDecl_ExportedE16normalInlineDeclEv(%struct.ClassTmpl* %this)
//
// MSC-DAG: define weak_odr dllexport                void @"\01?staticDef@?$ClassTmpl@UExplicitDecl_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInclass@?$ClassTmpl@UExplicitDecl_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDef@?$ClassTmpl@UExplicitDecl_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDecl@?$ClassTmpl@UExplicitDecl_Exported@@@@SAXXZ"()
// GNU-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitDecl_ExportedE9staticDefEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitDecl_ExportedE13staticInclassEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitDecl_ExportedE15staticInlineDefEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitDecl_ExportedE16staticInlineDeclEv()
//
// MSC-DAG: @"\01?StaticField@?$ClassTmpl@UExplicitDecl_Exported@@@@2HA"               = weak_odr dllexport global i32 1, align 4
// MSC-DAG: @"\01?StaticConstField@?$ClassTmpl@UExplicitDecl_Exported@@@@2HB"          = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ClassTmpl@UExplicitDecl_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ClassTmpl@UExplicitDecl_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?ConstexprField@?$ClassTmpl@UExplicitDecl_Exported@@@@2HB"            = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ExportedE11StaticFieldE                      = weak_odr dllexport global i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ExportedE16StaticConstFieldE                 = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ExportedE25StaticConstFieldEqualInitE        = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ExportedE25StaticConstFieldBraceInitE        = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitDecl_ExportedE14ConstexprFieldE                   = weak_odr dllexport constant i32 1, align 4
extern template struct __declspec(dllexport) ClassTmpl<ExplicitDecl_Exported>;
       template struct __declspec(dllexport) ClassTmpl<ExplicitDecl_Exported>;


// Export explicit instantiation definition of a non-exported class template.
//
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalDef@?$ClassTmpl@UExplicitInst_Exported@@@@QAEXXZ"(%struct.ClassTmpl.3* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalDef@?$ClassTmpl@UExplicitInst_Exported@@@@QEAAXXZ"(%struct.ClassTmpl.3* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInclass@?$ClassTmpl@UExplicitInst_Exported@@@@QAEXXZ"(%struct.ClassTmpl.3* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInclass@?$ClassTmpl@UExplicitInst_Exported@@@@QEAAXXZ"(%struct.ClassTmpl.3* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDef@?$ClassTmpl@UExplicitInst_Exported@@@@QAEXXZ"(%struct.ClassTmpl.3* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDef@?$ClassTmpl@UExplicitInst_Exported@@@@QEAAXXZ"(%struct.ClassTmpl.3* %this)
// M32-DAG: define weak_odr dllexport x86_thiscallcc void @"\01?normalInlineDecl@?$ClassTmpl@UExplicitInst_Exported@@@@QAEXXZ"(%struct.ClassTmpl.3* %this)
// M64-DAG: define weak_odr dllexport                void @"\01?normalInlineDecl@?$ClassTmpl@UExplicitInst_Exported@@@@QEAAXXZ"(%struct.ClassTmpl.3* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ExportedE9normalDefEv(%struct.ClassTmpl.3* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitInst_ExportedE9normalDefEv(%struct.ClassTmpl.3* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ExportedE13normalInclassEv(%struct.ClassTmpl.3* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitInst_ExportedE13normalInclassEv(%struct.ClassTmpl.3* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ExportedE15normalInlineDefEv(%struct.ClassTmpl.3* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitInst_ExportedE15normalInlineDefEv(%struct.ClassTmpl.3* %this)
// G32-DAG: define weak_odr dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitInst_ExportedE16normalInlineDeclEv(%struct.ClassTmpl.3* %this)
// G64-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitInst_ExportedE16normalInlineDeclEv(%struct.ClassTmpl.3* %this)
//
// MSC-DAG: define weak_odr dllexport                void @"\01?staticDef@?$ClassTmpl@UExplicitInst_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInclass@?$ClassTmpl@UExplicitInst_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDef@?$ClassTmpl@UExplicitInst_Exported@@@@SAXXZ"()
// MSC-DAG: define weak_odr dllexport                void @"\01?staticInlineDecl@?$ClassTmpl@UExplicitInst_Exported@@@@SAXXZ"()
// GNU-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitInst_ExportedE9staticDefEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitInst_ExportedE13staticInclassEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitInst_ExportedE15staticInlineDefEv()
// GNU-DAG: define weak_odr dllexport                void @_ZN9ClassTmplI21ExplicitInst_ExportedE16staticInlineDeclEv()
//
// MSC-DAG: @"\01?StaticField@?$ClassTmpl@UExplicitInst_Exported@@@@2HA"               = weak_odr dllexport global i32 1, align 4
// MSC-DAG: @"\01?StaticConstField@?$ClassTmpl@UExplicitInst_Exported@@@@2HB"          = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldEqualInit@?$ClassTmpl@UExplicitInst_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?StaticConstFieldBraceInit@?$ClassTmpl@UExplicitInst_Exported@@@@2HB" = weak_odr dllexport constant i32 1, align 4
// MSC-DAG: @"\01?ConstexprField@?$ClassTmpl@UExplicitInst_Exported@@@@2HB"            = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ExportedE11StaticFieldE                      = weak_odr dllexport global i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ExportedE16StaticConstFieldE                 = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ExportedE25StaticConstFieldEqualInitE        = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ExportedE25StaticConstFieldBraceInitE        = weak_odr dllexport constant i32 1, align 4
// GNU-DAG: @_ZN9ClassTmplI21ExplicitInst_ExportedE14ConstexprFieldE                   = weak_odr dllexport constant i32 1, align 4
template struct __declspec(dllexport) ClassTmpl<ExplicitInst_Exported>;


// Export specialization of a non-exported class template.
// M32-DAG: define dllexport x86_thiscallcc void @"\01?normalDef@?$ClassTmpl@UExplicitSpec_Exported@@@@QAEXXZ"(%struct.ClassTmpl.4* %this)
// M64-DAG: define dllexport                void @"\01?normalDef@?$ClassTmpl@UExplicitSpec_Exported@@@@QEAAXXZ"(%struct.ClassTmpl.4* %this)
// G32-DAG: define dllexport x86_thiscallcc void @_ZN9ClassTmplI21ExplicitSpec_ExportedE9normalDefEv(%struct.ClassTmpl.4* %this)
// G64-DAG: define dllexport                void @_ZN9ClassTmplI21ExplicitSpec_ExportedE9normalDefEv(%struct.ClassTmpl.4* %this)
template<>
struct __declspec(dllexport) ClassTmpl<ExplicitSpec_Exported>
{
  void normalDef();
};
void ClassTmpl<ExplicitSpec_Exported>::normalDef() {}
