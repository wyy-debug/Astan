using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using static System.Net.Mime.MediaTypeNames;
using System.Text;

namespace Sandbox
{ 
    class Dps
    {
        public dpsDamage parseProto(string protoName, Dictionary<string, object> protoData)
        {
            /* if (protoName == "SaveArchive")
            {
                // 这里可以添加具体逻辑
            }

            if (protoName == "KeepAlive")
            {
                // 这里可以添加具体逻辑
            }*/

            if (protoName == "damage")
            {
                string logJson = protoData["log"].ToString();
                var log = JsonConvert.DeserializeObject<Dictionary<string, object>>(logJson);

                string time                 = (string)protoData["time"];
                string WeaponName           = (string)log["WeaponName"];
                int Value                   = Convert.ToInt32(log["Value"]);
                int DamageValue             = Convert.ToInt32(log["DamageValue"]);
                int DamageKnockout          = Convert.ToInt32(log["DamageKnockout"]);
                int CritPercent             = Convert.ToInt32(log["CritPercent"]);
                int RandPercent             = Convert.ToInt32(log["RandPercent"]);
                int CritValue               = Convert.ToInt32(log["CritValue"]);
                string IsCrit               = (string)log["IsCrit"];
                int DamageChop              = Convert.ToInt32(log["DamageChop"]);
                string DamageKnockoutType   = (string)log["DamageKnockoutType"];
                int DamageRatio           = Convert.ToInt32(log["DamageRatio"]);
                if (WeaponName.Contains("Enemy"))
                {
                    Value = -Value;
                }
                
                try
                {
                    dpsDamage dps = new dpsDamage(
                    time,
                    WeaponName,
                    Value,
                    DamageValue,
                    DamageKnockout,
                    CritPercent,
                    RandPercent,
                    CritValue,
                    IsCrit,
                    DamageChop,
                    DamageKnockoutType,
                    DamageRatio);
                    return dps;

                }
                catch (Exception e)
                {
                    Console.WriteLine("error" + e);
                }
                
            }
            return null;
        }


    }

    class dpsDamage
    {
        public string Time { get; set; }
        public string WeaponName { get; set; }
        public int Value { get; set; }
        public int DamageValue { get; set; }
        public int DamageKnockout { get; set; }
        public int CritPercent { get; set; }
        public int RandPercent { get; set; }
        public int CritValue { get; set; }
        public string IsCrit { get; set; }
        public int DamageChop { get; set; }
        public string DamageKnockoutType { get; set; }
        public int DamageRatio { get; set; }

        public dpsDamage(string time, string weaponName, int value, int damageValue, int damageKnockout, int critPercent, int randPercent, int critValue, string isCrit, int damageChop, string damageKnockoutType, int damageRatio)
        {
            Time = time;
            WeaponName = weaponName;
            Value = value;
            DamageValue = damageValue;
            DamageKnockout = damageKnockout;
            CritPercent = critPercent;
            RandPercent = randPercent;
            CritValue = critValue;
            IsCrit = isCrit;
            DamageChop = damageChop;
            DamageKnockoutType = damageKnockoutType;
            DamageRatio = damageRatio;
        }
    }
}